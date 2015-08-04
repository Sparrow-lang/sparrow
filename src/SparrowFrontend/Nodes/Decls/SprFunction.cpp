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

#include <Nest/Frontend/SourceCode.h>

using namespace SprFrontend;
using namespace Feather;

namespace
{
    static const char* propIsMember = "spr.isMember";
}

SprFunction::SprFunction(const Location& loc, string name, Node* parameters, DynNode* returnType, DynNode* body, DynNode* ifClause, AccessType accessType)
    : DynNode(classNodeKind(), loc, {(DynNode*) parameters, returnType, body, ifClause})
{
    ASSERT( !parameters || parameters->nodeKind == nkFeatherNodeList );
    setName(node(), move(name));
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

Node* SprFunction::resultingFun() const
{
    return data_.explanation;
}

string SprFunction::toString() const
{
    ostringstream oss;
    oss << getName(node());
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
    addToSymTab(node());

    // If we don't have a children context, create one
    if ( !data_.childrenContext )
        data_.childrenContext = data_.context->createChildContext(node(), effectiveEvalMode(node()));

    DynNode::doSetContextForChildren();
}

void SprFunction::doComputeType()
{
    ASSERT(data_.children.size() == 4);
    Node* parameters = data_.children[0];
    DynNode* returnType = (DynNode*) data_.children[1];
    DynNode* body = (DynNode*) data_.children[2];
    DynNode* ifClause = (DynNode*) data_.children[3];

    bool isStatic = hasProperty(propIsStatic);

    // Check if this is a member function
    Node* parentClass = Feather::getParentClass(data_.context);
    bool isMember = nullptr != parentClass;
    if ( !isMember && isStatic )
        REP_ERROR(data_.location, "Only functions inside classes can be static");
    if ( isMember )
        setProperty(propIsMember, 1);

    // Is this a generic?
    if ( parameters )
    {
        Node* thisClass = isMember && !isStatic ? parentClass : nullptr;
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

    const string& funName = getName(node());

    // Special modifier for ctors & dtors
    if ( isMember && !isStatic && !hasProperty(propNoDefault) )
    {
        if ( funName == "ctor" )
            addModifier(new IntModCtorMembers);
        if ( funName == "dtor" )
            addModifier(new IntModDtorMembers);
    }

    EvalMode thisEvalMode = effectiveEvalMode(node());

    // Create the resulting function object
    Node* resultingFun = mkFunction(data_.location, funName, nullptr, {}, body->node());
    setShouldAddToSymTab(resultingFun, false);

    // Copy the "native" and the "autoCt" properties
    const string* nativeName = getPropertyString(propNativeName);
    if ( nativeName )
        Nest::setProperty(resultingFun, Feather::propNativeName, *nativeName);
    if ( hasProperty(propAutoCt) )
        Nest::setProperty(resultingFun, propAutoCt, 1);
    if ( hasProperty(propNoInline) )
        Nest::setProperty(resultingFun, propNoInline, 1);

    setEvalMode(resultingFun, thisEvalMode);
    resultingFun->childrenContext = data_.childrenContext;
    Nest::setContext(resultingFun, data_.context);
    setProperty(propResultingDecl, resultingFun);

    // Compute the types of the parameters first
    if ( parameters )
        Nest::computeType(parameters);

    // If this is a non-static member function, add this as a parameter
    if ( isMember && !isStatic )
    {
        TypeRef thisType = getDataType((Node*) parentClass, 1, thisEvalMode);
        Node* thisParam = Feather::mkVar(data_.location, "$this", mkTypeNode(data_.location, thisType));
        Nest::setContext(thisParam, data_.childrenContext);
        Function_addParameter(resultingFun, thisParam);
    }

    // Add the actual specified parameters
    if ( parameters )
    {
        for ( Node* n: parameters->children )
        {
            if ( !n )
                REP_ERROR(n->location, "Invalid node as parameter");

            Function_addParameter(resultingFun, n);
        }
    }

    // Compute the type of the return type node
    // We do this after the parameters, as the computation of the result might require access to the parameters
    TypeRef resType = returnType ? getType(returnType) : getVoidType(thisEvalMode);
    resType = adjustMode(resType, thisEvalMode, data_.childrenContext, data_.location);

    // If the parameter is a non-reference class, not basic numeric, add result parameter; otherwise, normal result
    if ( resType->hasStorage && resType->numReferences == 0 && !isBasicNumericType(resType) )
    {
        Node* resParam = Feather::mkVar(returnType->location(), "_result", mkTypeNode(returnType->location(), addRef(resType)));
        Nest::setContext(resParam, data_.childrenContext);
        Function_addParameter(resultingFun, resParam, true);
        Nest::setProperty(resultingFun, propResultParam, resParam);
        Function_setResultType(resultingFun, mkTypeNode(returnType->location(), getVoidType(thisEvalMode)));
    }
    else
        Function_setResultType(resultingFun, mkTypeNode(data_.location, resType));

    // TODO (explanation): explanation should be the result of semantic check
    data_.explanation = resultingFun;
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
        const string& funName = getName(node());
        
        if ( funName == "ctor" )
            handleStaticCtorDtor(true);
        if ( funName == "dtor" )
            handleStaticCtorDtor(false);
    }
}

void SprFunction::handleStaticCtorDtor(bool ctor)
{
    ASSERT(data_.children.size() == 4);
    Node* parameters = data_.children[0];

    // Make sure we don't have any parameters
    if ( parameters && !parameters->children.empty() )
        REP_ERROR(data_.location, "Static constructors and destructors cannot have parameters");

    // Add a global construct / destruct action call to this
    Node* funCall = mkFunCall(data_.location, data_.explanation, {});
    Node* n = ctor ? mkGlobalConstructAction(data_.location, funCall) : mkGlobalDestructAction(data_.location, funCall);
    Nest::setContext(n, data_.context);
    Nest::semanticCheck(n);
    ASSERT(data_.context->sourceCode());
    data_.context->sourceCode()->addAdditionalNode(n);
}
