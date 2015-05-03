#include <StdInc.h>
#include "SprVariable.h"
#include <NodeCommonsCpp.h>
#include <Helpers/SprTypeTraits.h>
#include <Helpers/DeclsHelpers.h>
#include <Helpers/CommonCode.h>
#include <Helpers/Convert.h>

#include <Feather/Util/Context.h>
#include <Feather/Util/Decl.h>

#include <Nest/Frontend/SourceCode.h>


using namespace SprFrontend;
using namespace Feather;

namespace
{
    enum VarKind
    {
        varLocal,
        varField,
        varGlobal,
    };

    Type* computeVarType(Node* parent, CompilationContext* ctx, Node* typeNode, Node* init)
    {
        const Location& loc = parent->location();

        Type* t = nullptr;

        // If a type node was given, take the type from it
        if ( typeNode )
        {
            t = getType(typeNode);
            if ( !t )
                REP_ERROR(loc, "Invalid type for variable");
        }
        else
        {
            // If no type node was given, maybe a type was given directly; if so, take it
            Type*const* givenType = parent->getPropertyType("spr.givenType");
            t = givenType ? *givenType : nullptr;
        }

        // Should we get the type from the initiailization expression?
        bool getTypeFromInit = nullptr == t;
        bool isRefAuto = false;
        if ( t && isConceptType(t, isRefAuto) )
            getTypeFromInit = true;
        if ( getTypeFromInit )
        {
            if ( !init )
                REP_ERROR(loc, "Initializer is requrired to deduce the type of the variable");

            init->computeType();

            // If still have a type (i.e, auto type), check for conversion
            if ( t && !canConvert(init, t) )
                REP_ERROR(init->location(), "Initializer of the variable (%1%) cannot be converted to variable type (%2%)")
                % init->type() % t;
            
            t = getAutoType(init, isRefAuto);
        }

        // Make sure we have the right mode for our context
        t = adjustMode(t, ctx, loc);
        return t;
    }
}

SprVariable::SprVariable(const Location& loc, string name, Node* typeNode, Node* init, AccessType accessType)
    : Node(loc, {typeNode, init})
{
    setName(this, move(name));
    setAccessType(this, accessType);
}

SprVariable::SprVariable(const Location& loc, string name, Type* type, Node* init, AccessType accessType)
    : Node(loc, {nullptr, init})
{
    setName(this, move(name));
    setAccessType(this, accessType);
    setProperty("spr.givenType", type);
}

void SprVariable::dump(ostream& os) const
{
    os << "var " << getName(this) << ": " << children_[0];
    if ( children_[1] )
        os << " = " << children_[1];
}

void SprVariable::doSetContextForChildren()
{
    addToSymTab(this);

    // Create a new child compilation context if the mode has changed; otherwise stay in the same context
    EvalMode curEvalMode = nodeEvalMode(this);
    if ( curEvalMode != modeUnspecified && curEvalMode != context_->evalMode() )
        childrenContext_ = new CompilationContext(context_, curEvalMode);
    else
        childrenContext_ = context_;

    Node::doSetContextForChildren();
}

void SprVariable::doComputeType()
{
    ASSERT(children_.size() == 2);
    Node* typeNode = children_[0];
    Node* init = children_[1];

    bool isStatic = hasProperty(propIsStatic);

    // Check the kind of the variable (local, global, field)
    VarKind varKind = varLocal;
    Function* parentFun = Feather::getParentFun(context_);
    Class* parentClass = nullptr;
    if ( !parentFun )
    {
        // Check if this is a member function
        parentClass = Feather::getParentClass(context_);
        if ( parentClass )
        {
            varKind = isStatic ? varGlobal : varField;
            if ( isStatic )
                parentClass = nullptr;
        }
        else
        {
            varKind = varGlobal;
            if ( isStatic )
                REP_ERROR(location_, "Only variables inside classes can be static");
        }
    }

    // Get the type of the variable
    Type* t = computeVarType(this, childrenContext_, typeNode, init);

    // If the type of the variable indicates a variable that can only be CT, change the evalMode
    if ( t->mode() == modeCt )
        setEvalMode(this, modeCt);

    // Create the resulting var
    Node* resultingVar = mkVar(location_, getName(this), mkTypeNode(location_, t));
    setEvalMode(resultingVar, effectiveEvalMode(this));
    setShouldAddToSymTab(resultingVar, false);
    this->setProperty(propResultingDecl, resultingVar);

    if ( varKind == varField )
    {
        resultingVar->setProperty(propIsField, 1);
    }

    resultingVar->setContext(childrenContext_);
    resultingVar->computeType();

    // If this is a CT variable in a non-ct function, make this a global variable
    if ( varKind == varLocal && context_->evalMode() == modeRt && isCt(t) )
        varKind = varGlobal;

    // If this is a CT variable in a non-ct function, make this a global variable

    bool isRef = t->noReferences() > 0;

    // Generate the initialization and destruction calls
    Node* ctorCall = nullptr;
    Node* dtorCall = nullptr;
    Node* varRef = nullptr;
    if ( varKind != varField && (init || !isRef) )
    {
        ASSERT(resultingVar->type());

        varRef = mkVarRef(location_, resultingVar);
        varRef->setContext(childrenContext_);

        if ( !isRef )
        {
            // Create ctor and dtor
            ctorCall = createCtorCall(location_, childrenContext_, varRef, init);
            if ( !Feather::isCt(resultingVar->type()) )
                dtorCall = createDtorCall(location_, childrenContext_, varRef);
        }
        else if ( init )   // Reference initialization
        {
            // Create an assignment operator
            ctorCall = mkOperatorCall(location_, varRef, ":=", init);
        }
    }

    // Set the explanation of this node
    Node* expl = nullptr;
    if ( varKind == varField )
    {
        // For fields, just explain this as the resulting var
        expl = resultingVar;
    }
    else
    {
        // For local and global variables take into consideration the ctor and dtor calls
        Node* resVar = resultingVar;
        if ( varKind == varLocal )
        {
            // For local variables, add the ctor & dtor actions in the node list, and make this as explanation
            dtorCall = dtorCall ? mkScopeDestructAction(location_, dtorCall) : nullptr;
        }
        else
        {
            // Add the variable at the top level
            ASSERT(context_->sourceCode());
            context_->sourceCode()->addAdditionalNode(resultingVar);
            resVar = nullptr;

            // For global variables, add the ctor & dtor actions as top level actions
            if ( ctorCall )
                ctorCall = mkGlobalConstructAction(location_, ctorCall);
            if ( dtorCall )
                dtorCall = mkGlobalDestructAction(location_, dtorCall);
        }
        expl = mkNodeList(location_, { resVar, ctorCall, dtorCall, mkNop(location_) });
    }

    ASSERT(expl);
    expl->setContext(childrenContext_);
    expl->computeType();
    explanation_ = expl;
    type_ = expl->type();

    setProperty("spr.resultingVar", resultingVar);

    // TODO (var): field initialization
    if ( init && varKind == varField )
        REP_ERROR(location_, "Initializers for class attributes are not supported yet");
}

void SprVariable::doSemanticCheck()
{
    computeType();

    // Semantically check the resulting variable and explanation
    Node* resultingVar = getCheckPropertyNode("spr.resultingVar");
    resultingVar->semanticCheck();
    explanation_->semanticCheck();
}
