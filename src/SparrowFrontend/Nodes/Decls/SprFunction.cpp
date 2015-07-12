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

SprFunction::SprFunction(const Location& loc, string name, NodeList* parameters, DynNode* returnType, DynNode* body, DynNode* ifClause, AccessType accessType)
    : DynNode(classNodeKind(), loc, {parameters, returnType, body, ifClause})
{
    setName(this, move(name));
    setAccessType(this, accessType);
}

bool SprFunction::hasThisParameters() const
{
    return hasProperty(propIsMember) && !hasProperty(propIsStatic);
}

DynNode* SprFunction::returnType() const
{
    ASSERT(data_.children.size() == 4);
    return (DynNode*) data_.children[1];
}
DynNode* SprFunction::body() const
{
    ASSERT(data_.children.size() == 4);
    return (DynNode*) data_.children[2];
}

Feather::Function* SprFunction::resultingFun() const
{
    return (Function*) data_.explanation;
}

string SprFunction::toString() const
{
    ostringstream oss;
    oss << getName(this);
    if ( data_.children[0] )
    {
        oss << '(';
        bool first = true;
        for ( auto pn: data_.children[0]->children )
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
    if ( data_.children[1] )
        os << ": " << data_.children[1];
    os << "\nbody: " << data_.children[2] << "\n";
}

void SprFunction::doSetContextForChildren()
{
    addToSymTab(this);

    // If we don't have a children context, create one
    if ( !data_.childrenContext )
        data_.childrenContext = data_.context->createChildContext(node(), effectiveEvalMode(this));

    DynNode::doSetContextForChildren();
}

void SprFunction::doComputeType()
{
    ASSERT(data_.children.size() == 4);
    NodeList* parameters = (NodeList*) data_.children[0];
    DynNode* returnType = (DynNode*) data_.children[1];
    DynNode* body = (DynNode*) data_.children[2];
    DynNode* ifClause = (DynNode*) data_.children[3];

    bool isStatic = hasProperty(propIsStatic);

    // Check if this is a member function
    Class* parentClass = Feather::getParentClass(data_.context);
    bool isMember = nullptr != parentClass;
    if ( !isMember && isStatic )
        REP_ERROR(data_.location, "Only functions inside classes can be static");
    if ( isMember )
        setProperty(propIsMember, 1);

    // Is this a generic?
    if ( parameters )
    {
        Class* thisClass = isMember && !isStatic ? parentClass : nullptr;
        DynNode* generic = GenericFunction::createGeneric(this, parameters, ifClause, thisClass);
        if ( generic )
        {
            // TODO (explanation): explanation should be the result of semantic check
            data_.explanation = generic->node();
            Nest::computeType(data_.explanation);
            data_.type = data_.explanation->type;
            setProperty(propResultingDecl, generic);
            return;
        }
    }
    if ( ifClause )
        REP_ERROR(data_.location, "If clauses must be applied only to generics; this is not a generic function");

    const string& funName = getName(this);

    // Special modifier for ctors & dtors
    if ( isMember && !isStatic && !hasProperty(propNoDefault) )
    {
        if ( funName == "ctor" )
            addModifier(new IntModCtorMembers);
        if ( funName == "dtor" )
            addModifier(new IntModDtorMembers);
    }

    EvalMode thisEvalMode = effectiveEvalMode(this);

    // Create the resulting function object
    Function* resultingFun = (Function*) mkFunction(data_.location, funName, nullptr, {}, body);
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
    resultingFun->setChildrenContext(data_.childrenContext);
    resultingFun->setContext(data_.context);
    setProperty(propResultingDecl, resultingFun);

    // Compute the types of the parameters first
    if ( parameters )
        parameters->computeType();

    // If this is a non-static member function, add this as a parameter
    if ( isMember && !isStatic )
    {
        TypeRef thisType = getDataType((Node*) parentClass, 1, thisEvalMode);
        DynNode* thisParam = Feather::mkVar(data_.location, "$this", mkTypeNode(data_.location, thisType));
        thisParam->setContext(data_.childrenContext);
        resultingFun->addParameter(thisParam);
    }

    // Add the actual specified parameters
    if ( parameters )
    {
        for ( DynNode* n: parameters->children() )
        {
            if ( !n )
                REP_ERROR(n->location(), "Invalid node as parameter");

            resultingFun->addParameter(n);
        }
    }

    // Compute the type of the return type node
    // We do this after the parameters, as the computation of the result might require access to the parameters
    TypeRef resType = returnType ? getType(returnType) : getVoidType(thisEvalMode);
    resType = adjustMode(resType, thisEvalMode, data_.childrenContext, data_.location);

    // If the parameter is a non-reference class, not basic numeric, add result parameter; otherwise, normal result
    if ( resType->hasStorage && resType->numReferences == 0 && !isBasicNumericType(resType) )
    {
        DynNode* resParam = Feather::mkVar(returnType->location(), "_result", mkTypeNode(returnType->location(), addRef(resType)));
        resParam->setContext(data_.childrenContext);
        resultingFun->addParameter(resParam, true);
        resultingFun->setProperty(propResultParam, resParam);
        resultingFun->setResultType(mkTypeNode(returnType->location(), getVoidType(thisEvalMode)));
    }
    else
        resultingFun->setResultType(mkTypeNode(data_.location, resType));

    // TODO (explanation): explanation should be the result of semantic check
    data_.explanation = resultingFun->node();
    Nest::computeType(data_.explanation);
    data_.type = data_.explanation->type;

    // Check for Std functions
    checkStdFunction(this);
}

void SprFunction::doSemanticCheck()
{
    computeType();

    ASSERT(data_.explanation);
    Nest::semanticCheck(data_.explanation);

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
    ASSERT(data_.children.size() == 4);
    NodeList* parameters = (NodeList*) data_.children[0];

    // Make sure we don't have any parameters
    if ( parameters && !parameters->children().empty() )
        REP_ERROR(data_.location, "Static constructors and destructors cannot have parameters");

    // Add a global construct / destruct action call to this
    DynNode* funCall = mkFunCall(data_.location, (DynNode*) data_.explanation, {});
    DynNode* n = ctor ? mkGlobalConstructAction(data_.location, funCall) : mkGlobalDestructAction(data_.location, funCall);
    n->setContext(data_.context);
    n->semanticCheck();
    ASSERT(data_.context->sourceCode());
    data_.context->sourceCode()->addAdditionalNode(n);
}
