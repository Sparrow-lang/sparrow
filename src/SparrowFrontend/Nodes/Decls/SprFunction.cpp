#include <StdInc.h>
#include "SprFunction.h"
#include "GenericFunction.h"
#include <NodeCommonsCpp.h>
#include <Helpers/SprTypeTraits.h>
#include <Helpers/StdDef.h>
#include <Helpers/DeclsHelpers.h>
#include <IntMods/IntModDtorMembers.h>
#include <IntMods/IntModCtorMembers.h>

#include <Feather/Util/Context.h>
#include <Feather/Util/TypeTraits.h>
#include <Feather/Util/Decl.h>
#include <Feather/Nodes/Decls/Function.h>

#include <Nest/Frontend/SourceCode.h>

using namespace SprFrontend;
using namespace Feather;

namespace
{
    static const char* propIsMember = "spr.isMember";
}

SprFunction::SprFunction(const Location& loc, string name, NodeList* parameters, Node* returnType, Node* body, Node* ifClause, AccessType accessType)
    : Node(classNodeKind(), loc, {parameters, returnType, body, ifClause})
{
    setName(this, move(name));
    setAccessType(this, accessType);
}

bool SprFunction::hasThisParameters() const
{
    return hasProperty(propIsMember) && !hasProperty(propIsStatic);
}

Node* SprFunction::returnType() const
{
    ASSERT(children_.size() == 4);
    return children_[1];
}
Node* SprFunction::body() const
{
    ASSERT(children_.size() == 4);
    return children_[2];
}

Feather::Function* SprFunction::resultingFun() const
{
    return explanation_->as<Function>();
}

string SprFunction::toString() const
{
    ostringstream oss;
    oss << getName(this);
    if ( children_[0] )
    {
        oss << '(';
        bool first = true;
        for ( auto pn: children_[0]->children() )
        {
            if ( first )
                first = false;
            else
                oss << ", ";

            oss << pn;
        }
        oss << ')';
    }
    return oss.str();
}

void SprFunction::dump(ostream& os) const
{
    os << "fun " << toString();
    if ( children_[1] )
        os << ": " << children_[1];
    os << "\nbody: " << children_[2] << "\n";
}

void SprFunction::doSetContextForChildren()
{
    addToSymTab(this);

    // If we don't have a children context, create one
    if ( !childrenContext_ )
        childrenContext_ = context_->createChildContext(this, effectiveEvalMode(this));

    Node::doSetContextForChildren();
}

void SprFunction::doComputeType()
{
    ASSERT(children_.size() == 4);
    NodeList* parameters = (NodeList*) children_[0];
    Node* returnType = children_[1];
    Node* body = children_[2];
    Node* ifClause = children_[3];

    bool isStatic = hasProperty(propIsStatic);

    // Check if this is a member function
    Class* parentClass = Feather::getParentClass(context_);
    bool isMember = nullptr != parentClass;
    if ( !isMember && isStatic )
        REP_ERROR(location_, "Only functions inside classes can be static");
    if ( isMember )
        setProperty(propIsMember, 1);

    // Is this a generic?
    if ( parameters )
    {
        Class* thisClass = isMember && !isStatic ? parentClass : nullptr;
        Node* generic = GenericFunction::createGeneric(this, parameters, ifClause, thisClass);
        if ( generic )
        {
            // TODO (explanation): explanation should be the result of semantic check
            explanation_ = generic;
            explanation_->computeType();
            type_ = explanation_->type();
            setProperty(propResultingDecl, generic);
            return;
        }
    }
    if ( ifClause )
        REP_ERROR(location_, "If clauses must be applied only to generics; this is not a generic function");

    const string& funName = getName(this);

    // Special modifier for ctors & dtors
    if ( isMember && !isStatic && !hasProperty(propNoDefault) )
    {
        if ( funName == "ctor" )
            modifiers_.push_back(new IntModCtorMembers);
        if ( funName == "dtor" )
            modifiers_.push_back(new IntModDtorMembers);
    }

    EvalMode thisEvalMode = effectiveEvalMode(this);

    // Create the resulting function object
    Function* resultingFun = (Function*) mkFunction(location_, funName, nullptr, {}, body);
    setShouldAddToSymTab(resultingFun, false);

    // Copy the "native" and the "autoCt" properties
    const string* nativeName = getPropertyString(propNativeName);
    if ( nativeName )
        resultingFun->setProperty(Feather::propNativeName, *nativeName);
    if ( hasProperty(propAutoCt) )
        resultingFun->setProperty(propAutoCt, 1);
    if ( hasProperty(propNoInline) )
        resultingFun->setProperty(propNoInline, 1);

    setEvalMode(resultingFun, thisEvalMode);
    resultingFun->setChildrenContext(childrenContext_);
    resultingFun->setContext(context_);
    setProperty(propResultingDecl, resultingFun);

    // Compute the types of the parameters first
    if ( parameters )
        parameters->computeType();

    // If this is a non-static member function, add this as a parameter
    if ( isMember && !isStatic )
    {
        TypeRef thisType = getDataType(parentClass, 1, thisEvalMode);
        Node* thisParam = Feather::mkVar(location_, "$this", mkTypeNode(location_, thisType));
        thisParam->setContext(childrenContext_);
        resultingFun->addParameter(thisParam);
    }

    // Add the actual specified parameters
    if ( parameters )
    {
        for ( Node* n: parameters->children() )
        {
            if ( !n )
                REP_ERROR(n->location(), "Invalid node as parameter");

            resultingFun->addParameter(n);
        }
    }

    // Compute the type of the return type node
    // We do this after the parameters, as the computation of the result might require access to the parameters
    TypeRef resType = returnType ? getType(returnType) : getVoidType(thisEvalMode);
    resType = adjustMode(resType, thisEvalMode, childrenContext_, location_);

    // If the parameter is a non-reference class, not basic numeric, add result parameter; otherwise, normal result
    if ( resType->hasStorage && resType->numReferences == 0 && !isBasicNumericType(resType) )
    {
        Node* resParam = Feather::mkVar(returnType->location(), "_result", mkTypeNode(returnType->location(), addRef(resType)));
        resParam->setContext(childrenContext_);
        resultingFun->addParameter(resParam, true);
        resultingFun->setProperty(propResultParam, resParam);
        resultingFun->setResultType(mkTypeNode(returnType->location(), getVoidType(thisEvalMode)));
    }
    else
        resultingFun->setResultType(mkTypeNode(location_, resType));

    // TODO (explanation): explanation should be the result of semantic check
    explanation_ = resultingFun;
    explanation_->computeType();
    type_ = explanation_->type();

    // Check for Std functions
    checkStdFunction(this);
}

void SprFunction::doSemanticCheck()
{
    computeType();

    ASSERT(explanation_);
    explanation_->semanticCheck();

    // Check for static ctors & dtors
    if ( resultingFun() && (!hasProperty(propIsMember) || hasProperty(propIsStatic)) )
    {
        const string& funName = getName(this);
        
        if ( funName == "ctor" )
            handleStaticCtorDtor(true);
        if ( funName == "dtor" )
            handleStaticCtorDtor(false);
    }
}

void SprFunction::handleStaticCtorDtor(bool ctor)
{
    ASSERT(children_.size() == 4);
    NodeList* parameters = (NodeList*) children_[0];

    // Make sure we don't have any parameters
    if ( parameters && !parameters->children().empty() )
        REP_ERROR(location_, "Static constructors and destructors cannot have parameters");

    // Add a global construct / destruct action call to this
    Node* funCall = mkFunCall(location_, explanation_, {});
    Node* n = ctor ? mkGlobalConstructAction(location_, funCall) : mkGlobalDestructAction(location_, funCall);
    n->setContext(context_);
    n->semanticCheck();
    ASSERT(context_->sourceCode());
    context_->sourceCode()->addAdditionalNode(n);
}
