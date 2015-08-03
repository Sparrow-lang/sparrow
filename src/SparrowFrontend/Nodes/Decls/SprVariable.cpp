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

    TypeRef computeVarType(DynNode* parent, CompilationContext* ctx, DynNode* typeNode, DynNode* init)
    {
        const Location& loc = parent->location();

        TypeRef t = nullptr;

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
            const TypeRef* givenType = parent->getPropertyType("spr.givenType");
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

SprVariable::SprVariable(const Location& loc, string name, DynNode* typeNode, DynNode* init, AccessType accessType)
    : DynNode(classNodeKind(), loc, {typeNode, init})
{
    setName(node(), move(name));
    setAccessType(this, accessType);
}

SprVariable::SprVariable(const Location& loc, string name, TypeRef type, DynNode* init, AccessType accessType)
    : DynNode(classNodeKind(), loc, {nullptr, init})
{
    setName(node(), move(name));
    setAccessType(this, accessType);
    setProperty("spr.givenType", type);
}

void SprVariable::dump(ostream& os) const
{
    os << "var " << getName(node()) << ": " << data_.children[0];
    if ( data_.children[1] )
        os << " = " << data_.children[1];
}

void SprVariable::doSetContextForChildren()
{
    addToSymTab(node());

    // Create a new child compilation context if the mode has changed; otherwise stay in the same context
    EvalMode curEvalMode = nodeEvalMode(node());
    if ( curEvalMode != modeUnspecified && curEvalMode != data_.context->evalMode() )
        data_.childrenContext = new CompilationContext(data_.context, curEvalMode);
    else
        data_.childrenContext = data_.context;

    DynNode::doSetContextForChildren();
}

void SprVariable::doComputeType()
{
    ASSERT(data_.children.size() == 2);
    DynNode* typeNode = (DynNode*) data_.children[0];
    DynNode* init = (DynNode*) data_.children[1];

    bool isStatic = hasProperty(propIsStatic);

    // Check the kind of the variable (local, global, field)
    VarKind varKind = varLocal;
    Node* parentFun = Feather::getParentFun(data_.context);
    Class* parentClass = nullptr;
    if ( !parentFun )
    {
        // Check if this is a member function
        parentClass = Feather::getParentClass(data_.context);
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
                REP_ERROR(data_.location, "Only variables inside classes can be static");
        }
    }

    // Get the type of the variable
    TypeRef t = computeVarType(this, data_.childrenContext, typeNode, init);

    // If the type of the variable indicates a variable that can only be CT, change the evalMode
    if ( t->mode == modeCt )
        setEvalMode(node(), modeCt);

    // Create the resulting var
    Node* resultingVar = mkVar(data_.location, getName(node()), mkTypeNode(data_.location, t));
    setEvalMode(resultingVar, effectiveEvalMode(node()));
    setShouldAddToSymTab(resultingVar, false);
    this->setProperty(propResultingDecl, (DynNode*) resultingVar);

    if ( varKind == varField )
    {
        Nest::setProperty(resultingVar, propIsField, 1);
    }

    Nest::setContext(resultingVar, data_.childrenContext);
    Nest::computeType(resultingVar);

    // If this is a CT variable in a non-ct function, make this a global variable
    if ( varKind == varLocal && data_.context->evalMode() == modeRt && isCt(t) )
        varKind = varGlobal;

    // If this is a CT variable in a non-ct function, make this a global variable

    bool isRef = t->numReferences > 0;

    // Generate the initialization and destruction calls
    DynNode* ctorCall = nullptr;
    DynNode* dtorCall = nullptr;
    DynNode* varRef = nullptr;
    if ( varKind != varField && (init || !isRef) )
    {
        ASSERT(resultingVar->type);

        varRef = (DynNode*) mkVarRef(data_.location, resultingVar);
        varRef->setContext(data_.childrenContext);

        if ( !isRef )
        {
            // Create ctor and dtor
            ctorCall = createCtorCall(data_.location, data_.childrenContext, varRef, init);
            if ( !Feather::isCt(resultingVar->type) )
                dtorCall = createDtorCall(data_.location, data_.childrenContext, varRef);
        }
        else if ( init )   // Reference initialization
        {
            // Create an assignment operator
            ctorCall = (DynNode*) mkOperatorCall(data_.location, varRef->node(), ":=", init->node());
        }
    }

    // Set the explanation of this node
    DynNode* expl = nullptr;
    if ( varKind == varField )
    {
        // For fields, just explain this as the resulting var
        expl = (DynNode*) resultingVar;
    }
    else
    {
        // For local and global variables take into consideration the ctor and dtor calls
        DynNode* resVar = (DynNode*) resultingVar;
        if ( varKind == varLocal )
        {
            // For local variables, add the ctor & dtor actions in the node list, and make this as explanation
            dtorCall = dtorCall ? (DynNode*) mkScopeDestructAction(data_.location, dtorCall->node()) : nullptr;
        }
        else
        {
            // Add the variable at the top level
            ASSERT(data_.context->sourceCode());
            data_.context->sourceCode()->addAdditionalNode(resultingVar);
            resVar = nullptr;

            // For global variables, add the ctor & dtor actions as top level actions
            if ( ctorCall )
                ctorCall = (DynNode*) mkGlobalConstructAction(data_.location, ctorCall->node());
            if ( dtorCall )
                dtorCall = (DynNode*) mkGlobalDestructAction(data_.location, dtorCall->node());
        }
        expl = (DynNode*) mkNodeList(data_.location, { resVar->node(), ctorCall->node(), dtorCall->node(), mkNop(data_.location) });
    }

    ASSERT(expl);
    expl->setContext(data_.childrenContext);
    expl->computeType();
    data_.explanation = expl->node();
    data_.type = expl->type();

    setProperty("spr.resultingVar", resultingVar);

    // TODO (var): field initialization
    if ( init && varKind == varField )
        REP_ERROR(data_.location, "Initializers for class attributes are not supported yet");
}

void SprVariable::doSemanticCheck()
{
    computeType();

    // Semantically check the resulting variable and explanation
    DynNode* resultingVar = getCheckPropertyDynNode("spr.resultingVar");
    resultingVar->semanticCheck();
    Nest::semanticCheck(data_.explanation);
}
