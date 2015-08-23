#include <StdInc.h>
#include "SparrowNodes.h"
#include "SparrowNodesAccessors.h"

#include "Mods/ModStatic.h"
#include "Mods/ModCt.h"
#include "Mods/ModRt.h"
#include "Mods/ModRtCt.h"
#include "Mods/ModAutoCt.h"
#include "Mods/ModCtGeneric.h"
#include "Mods/ModNative.h"
#include "Mods/ModConvert.h"
#include "Mods/ModNoDefault.h"
#include "Mods/ModInitCtor.h"
#include "Mods/ModMacro.h"
#include "Mods/ModNoInline.h"

#include <Helpers/ForEachNodeInNodeList.h>
#include <Helpers/DeclsHelpers.h>
#include <Helpers/SprTypeTraits.h>
#include <Helpers/Convert.h>
#include <Helpers/CommonCode.h>


#include <Feather/Nodes/FeatherNodes.h>
#include <Feather/Util/Decl.h>
#include <Feather/Util/TypeTraits.h>
#include <Feather/Util/Context.h>

#include <Nest/Common/Diagnostic.h>
#include <Nest/Intermediate/NodeKindRegistrar.h>


using namespace SprFrontend;
using namespace Feather;
using namespace Nest;

#define UNUSED(x) (void)(x)

////////////////////////////////////////////////////////////////////////////////
// ModifiersNode
//

void applyModifier(Node* base, Node* modNode)
{
    Nest::Modifier* mod = nullptr;
    
    if ( modNode->nodeKind == nkSparrowExpIdentifier )
    {
        const string& name = getCheckPropertyString(modNode, "name");
        if ( name == "static" )
            mod = new ModStatic;
        else if ( name == "ct" )
            mod = new ModCt;
        else if ( name == "rt" )
            mod = new ModRt;
        else if ( name == "rtct" )
            mod = new ModRtCt;
        else if ( name == "autoCt" )
            mod = new ModAutoCt;
        else if ( name == "ctGeneric" )
            mod = new ModCtGeneric;
        else if ( name == "convert" )
            mod = new ModConvert;
        else if ( name == "noDefault" )
            mod = new ModNoDefault;
        else if ( name == "initCtor" )
            mod = new ModInitCtor;
        else if ( name == "macro" )
            mod = new ModMacro;
        else if ( name == "noInline" )
            mod = new ModNoInline;
    }
    else
    {
        // check for: native("name")
        if ( modNode->nodeKind == nkSparrowExpInfixExp )    // fun application
        {
            Node* fbase = modNode->children[0];
            Node* fargs = modNode->children[1];
            if ( fbase->nodeKind == nkSparrowExpIdentifier && getCheckPropertyString(fbase, "name") == "native" )
            {
                // one argument: a literal
                if ( fargs && fargs->nodeKind == Feather::nkFeatherNodeList && fargs->children.size() == 1
                && fargs->children.front()->nodeKind == nkSparrowExpLiteral )
                {
                    Node* arg = fargs->children.front();
                    if ( Literal_isString(arg) )
                    {
                        mod = new ModNative(Literal_getData(arg));
                    }
                }
            }
        }
    }
    
    // If we recognized a modifier, add it to the base node; otherwise raise an error
    if ( mod )
        Nest::addModifier(base, mod);
    else
        REP_ERROR(modNode->location, "Unknown modifier found: %1%") % modNode;
}
void ModifiersNode_SetContextForChildren(Node* node)
{
    Node* base = node->children[0];
    Node* modifierNodes = node->children[1];

    // Set the context of the modifiers
    if ( modifierNodes )
        Nest::setContext(modifierNodes, node->context);

    // Interpret the modifiers here - as much as possible
    if ( modifierNodes )
    {
        if ( modifierNodes->nodeKind == Feather::nkFeatherNodeList )
        {
            // Add the modifiers to the base node
            forEachNodeInNodeList(modifierNodes, [&] (Node* modNode)
            {
                applyModifier(base, modNode);
            });
        }
        else
        {
            applyModifier(base, modifierNodes);
        }
    }

    // Set the context for the base
    if ( base )
        Nest::setContext(base, node->context);
}
TypeRef ModifiersNode_ComputeType(Node* node)
{
    Node* base = node->children[0];
    computeType(base);
    return base->type;
}
Node* ModifiersNode_SemanticCheck(Node* node)
{
    return node->children[0];
}


////////////////////////////////////////////////////////////////////////////////
// Instantiation
//

Node* Instantiation_SemanticCheck(Node* node)
{
    return mkNop(node->location);
}


////////////////////////////////////////////////////////////////////////////////
// InstantiationSet
//

Node* InstantiationsSet_SemanticCheck(Node* node)
{
    return mkNop(node->location);
}


////////////////////////////////////////////////////////////////////////////////
// For
//

void For_SetContextForChildren(Node* node)
{
    ASSERT(node->children.size() == 3);
    Node* range = node->children[0];
    Node* action = node->children[1];
    Node* typeExpr = node->children[2];

    ASSERT(range);
    CompilationContext* rangeContext = nodeEvalMode(node) == modeCt ? Nest_mkChildContext(node->context, modeCt) : node->context;
    if ( typeExpr )
        Nest::setContext(typeExpr, rangeContext);
    Nest::setContext(range, rangeContext);
    if ( action )
        Nest::setContext(action, node->context);
}

TypeRef For_ComputeType(Node* node)
{
    return getVoidType(node->context->evalMode);
}

Node* For_SemanticCheck(Node* node)
{
    ASSERT(node->children.size() == 3);
    Node* range = node->children[0];
    Node* action = node->children[1];
    Node* typeExpr = node->children[2];

    bool ctFor = nodeEvalMode(node) == modeCt;

    if ( typeExpr )
        Nest::semanticCheck(typeExpr);
    Nest::semanticCheck(range);

    const Location& loc = range->location;

    if ( ctFor && !isCt(range->type) )
        REP_ERROR(loc, "Range must be available at CT, for a CT for (range type: %1%)") % range->type;

    // Expand the for statement of the form
    //      for ( <name>: <type> = <range> ) action;
    //
    // into:
    //      var $rangeVar: Range = <range>;
    //      while ( ! $rangeVar.isEmpty() ; $rangeVar.popFront() )
    //      {
    //          var <name>: <type> = $rangeVar.front();
    //          action;
    //      }
    //
    // if <type> is not present, we will use '$rangeType.RetType'

    // Variable to hold the range - initialize it with the range node
    Node* rangeVar = mkSprVariable(loc, "$rangeVar", mkIdentifier(loc, "Range"), range);
    if ( ctFor )
        setEvalMode(rangeVar, modeCt);
    Node* rangeVarRef = mkIdentifier(loc, "$rangeVar");

    // while condition
    Node* base1 = mkCompoundExp(loc, rangeVarRef, "isEmpty");
    Node* whileCond = mkOperatorCall(loc, nullptr, "!", mkFunApplication(loc, base1, nullptr));

    // while step
    Node* base2 = mkCompoundExp(loc, rangeVarRef, "popFront");
    Node* whileStep = mkFunApplication(loc, base2, nullptr);

    // while body
    Node* whileBody = nullptr;
    if ( action )
    {
        if ( !typeExpr )
            typeExpr = mkCompoundExp(loc, rangeVarRef, "RetType");

        // the iteration variable
        Node* base3 = mkCompoundExp(loc, rangeVarRef, "front");
        Node* init = mkFunApplication(loc, base3, nullptr);

        Node* iterVar = mkSprVariable(node->location, getName(node), typeExpr, init);
        if ( ctFor )
            setEvalMode(iterVar, modeCt);

        whileBody = mkLocalSpace(action->location, { iterVar, action });
    }

    Node* whileStmt = mkWhile(loc, whileCond, whileBody, whileStep, ctFor);
    
    return mkLocalSpace(node->location, { rangeVar, whileStmt });
}


////////////////////////////////////////////////////////////////////////////////
// SprReturn
//

Node* SprReturn_SemanticCheck(Node* node)
{
    ASSERT(node->children.size() == 1);
    Node* exp = node->children[0];

    // Get the parent function of this return
    Node* parentFun = getParentFun(node->context);
    if ( !parentFun )
        REP_ERROR(node->location, "Return found outside any function");

    // Compute the result type of the function
    TypeRef resType = nullptr;
    Node* resultParam = getResultParam(parentFun);
    if ( resultParam ) // Does this function have a result param?
    {
        resType = removeRef(resultParam->type);
        ASSERT(!Function_resultType(parentFun)->hasStorage); // The function should have void result
    }
    else
    {
        resType = Function_resultType(parentFun);
    }
    ASSERT(resType);

    // Check match between return expression and function result type
    ConversionResult cvt = convNone;
    if ( exp )
    {
        Nest::semanticCheck(exp);
        if ( !resType->hasStorage && exp->type == resType )
        {
            return mkNodeList(node->location, { exp, mkReturn(node->location) });
        }
        else
        {
            cvt = canConvert(exp, resType);
        }
        if ( !cvt )
            REP_ERROR(exp->location, "Cannot convert return expression (%1%) to %2%") % exp->type % resType;
    }
    else
    {
        if ( Function_resultType(parentFun)->typeKind != typeKindVoid )
            REP_ERROR(node->location, "You must return something in a function that has non-Void result type");
    }

    // Build the explanation of this node
    if ( resultParam )
    {
        // Create a ctor to construct the result parameter with the expression received
        const Location& l = resultParam->location;
        Node* thisArg = mkMemLoad(l, mkVarRef(l, resultParam));
        Nest::setContext(thisArg, node->context);
        Node* action = createCtorCall(l, node->context, thisArg, exp);
        if ( !action )
            REP_ERROR(exp->location, "Cannot construct return type object %1% from %2%") % exp->type % resType;

        return mkNodeList(node->location, { action, mkReturn(node->location, nullptr)});
    }
    else
    {
        exp = exp ? cvt.apply(node->context, exp) : nullptr;
        return mkReturn(node->location, exp);
    }
}



////////////////////////////////////////////////////////////////////////////////
// Function forwards - implemented in SparrowNodes_Decl.cpp
//

void SprCompilationUnit_SetContextForChildren(Node* node);
TypeRef SprCompilationUnit_ComputeType(Node* node);
Node* SprCompilationUnit_SemanticCheck(Node* node);

void Package_SetContextForChildren(Node* node);
TypeRef Package_ComputeType(Node* node);
Node* Package_SemanticCheck(Node* node);

void SprClass_SetContextForChildren(Node* node);
TypeRef SprClass_ComputeType(Node* node);
Node* SprClass_SemanticCheck(Node* node);

void SprFunction_SetContextForChildren(Node* node);
TypeRef SprFunction_ComputeType(Node* node);
Node* SprFunction_SemanticCheck(Node* node);

void SprParameter_SetContextForChildren(Node* node);
TypeRef SprParameter_ComputeType(Node* node);
Node* SprParameter_SemanticCheck(Node* node);

void SprVariable_SetContextForChildren(Node* node);
TypeRef SprVariable_ComputeType(Node* node);
Node* SprVariable_SemanticCheck(Node* node);

void SprConcept_SetContextForChildren(Node* node);
Node* SprConcept_SemanticCheck(Node* node);

Node* Generic_SemanticCheck(Node* node);

void Using_SetContextForChildren(Node* node);
TypeRef Using_ComputeType(Node* node);
Node* Using_SemanticCheck(Node* node);

////////////////////////////////////////////////////////////////////////////////
// Function forwards - implemented in SparrowNodes_Exp.cpp
//

Node* Literal_SemanticCheck(Node* node);
Node* This_SemanticCheck(Node* node);
Node* Identifier_SemanticCheck(Node* node);
Node* CompoundExp_SemanticCheck(Node* node);
Node* FunApplication_SemanticCheck(Node* node);
Node* OperatorCall_SemanticCheck(Node* node);
Node* InfixExp_SemanticCheck(Node* node);
Node* LambdaFunction_SemanticCheck(Node* node);
Node* SprConditional_SemanticCheck(Node* node);
Node* DeclExp_SemanticCheck(Node* node);
Node* StarExp_SemanticCheck(Node* node);



////////////////////////////////////////////////////////////////////////////////
// Node kind variables
//

int SprFrontend::firstSparrowNodeKind = 0;

int SprFrontend::nkSparrowModifiersNode = 0;

int SprFrontend::nkSparrowDeclSprCompilationUnit = 0;
int SprFrontend::nkSparrowDeclPackage = 0;
int SprFrontend::nkSparrowDeclSprClass = 0;
int SprFrontend::nkSparrowDeclSprFunction = 0;
int SprFrontend::nkSparrowDeclSprParameter = 0;
int SprFrontend::nkSparrowDeclSprVariable = 0;
int SprFrontend::nkSparrowDeclSprConcept = 0;
int SprFrontend::nkSparrowDeclGenericClass = 0;
int SprFrontend::nkSparrowDeclGenericFunction = 0;
int SprFrontend::nkSparrowDeclUsing = 0;

int SprFrontend::nkSparrowExpLiteral = 0;
int SprFrontend::nkSparrowExpThis = 0;
int SprFrontend::nkSparrowExpIdentifier = 0;
int SprFrontend::nkSparrowExpCompoundExp = 0;
int SprFrontend::nkSparrowExpFunApplication = 0;
int SprFrontend::nkSparrowExpOperatorCall = 0;
int SprFrontend::nkSparrowExpInfixExp = 0;
int SprFrontend::nkSparrowExpLambdaFunction = 0;
int SprFrontend::nkSparrowExpSprConditional = 0;
int SprFrontend::nkSparrowExpDeclExp = 0;
int SprFrontend::nkSparrowExpStarExp = 0;

int SprFrontend::nkSparrowStmtFor = 0;
int SprFrontend::nkSparrowStmtSprReturn = 0;

int SprFrontend::nkSparrowInnerInstantiation = 0;
int SprFrontend::nkSparrowInnerInstantiationsSet = 0;

////////////////////////////////////////////////////////////////////////////////
// Functions from the header
//

void SprFrontend::initSparrowNodeKinds()
{
    nkSparrowModifiersNode =            registerNodeKind("spr.modifiers", &ModifiersNode_SemanticCheck, &ModifiersNode_ComputeType, &ModifiersNode_SetContextForChildren, NULL);

    nkSparrowDeclSprCompilationUnit =   registerNodeKind("spr.sprCompilationUnit", &SprCompilationUnit_SemanticCheck, &SprCompilationUnit_ComputeType, &SprCompilationUnit_SetContextForChildren, NULL);
    nkSparrowDeclPackage =              registerNodeKind("spr.package", &Package_SemanticCheck, &Package_ComputeType, &Package_SetContextForChildren, NULL);
    nkSparrowDeclSprClass =             registerNodeKind("spr.sprClass", &SprClass_SemanticCheck, &SprClass_ComputeType, &SprClass_SetContextForChildren, NULL);
    nkSparrowDeclSprFunction =          registerNodeKind("spr.sprFunction", &SprFunction_SemanticCheck, &SprFunction_ComputeType, &SprFunction_SetContextForChildren, NULL);
    nkSparrowDeclSprParameter =         registerNodeKind("spr.sprParameter", &SprParameter_SemanticCheck, &SprParameter_ComputeType, &SprParameter_SetContextForChildren, NULL);
    nkSparrowDeclSprVariable =          registerNodeKind("spr.sprVariable", &SprVariable_SemanticCheck, &SprVariable_ComputeType, &SprVariable_SetContextForChildren, NULL);
    nkSparrowDeclSprConcept =           registerNodeKind("spr.sprConcept", &SprConcept_SemanticCheck, NULL, &SprConcept_SetContextForChildren, NULL);
    nkSparrowDeclGenericClass =         registerNodeKind("spr.genericClass", &Generic_SemanticCheck, NULL, NULL, NULL);
    nkSparrowDeclGenericFunction =      registerNodeKind("spr.genericFunction", &Generic_SemanticCheck, NULL, NULL, NULL);
    nkSparrowDeclUsing =                registerNodeKind("spr.using", &Using_SemanticCheck, &Using_ComputeType, &Using_SetContextForChildren, NULL);

    nkSparrowExpLiteral =               registerNodeKind("spr.literal", &Literal_SemanticCheck, NULL, NULL, NULL);
    nkSparrowExpThis =                  registerNodeKind("spr.this", &This_SemanticCheck, NULL, NULL, NULL);
    nkSparrowExpIdentifier =            registerNodeKind("spr.identifier", &Identifier_SemanticCheck, NULL, NULL, NULL);
    nkSparrowExpCompoundExp =           registerNodeKind("spr.compoundExp", &CompoundExp_SemanticCheck, NULL, NULL, NULL);
    nkSparrowExpFunApplication =        registerNodeKind("spr.funApplication", &FunApplication_SemanticCheck, NULL, NULL, NULL);
    nkSparrowExpOperatorCall =          registerNodeKind("spr.operatorCall", &OperatorCall_SemanticCheck, NULL, NULL, NULL);
    nkSparrowExpInfixExp =              registerNodeKind("spr.infixExp", &InfixExp_SemanticCheck, NULL, NULL, NULL);
    nkSparrowExpLambdaFunction =        registerNodeKind("spr.lambdaFunction", &LambdaFunction_SemanticCheck, NULL, NULL, NULL);
    nkSparrowExpSprConditional =        registerNodeKind("spr.sprConditional", &SprConditional_SemanticCheck, NULL, NULL, NULL);
    nkSparrowExpDeclExp =               registerNodeKind("spr.declExp", &DeclExp_SemanticCheck, NULL, NULL, NULL);
    nkSparrowExpStarExp =               registerNodeKind("spr.starExp", &StarExp_SemanticCheck, NULL, NULL, NULL);
    
    nkSparrowStmtFor =                  registerNodeKind("spr.for", &For_SemanticCheck, &For_ComputeType, &For_SetContextForChildren, NULL);
    nkSparrowStmtSprReturn =            registerNodeKind("spr.return", &SprReturn_SemanticCheck, NULL, NULL, NULL);

    nkSparrowInnerInstantiation =       registerNodeKind("spr.instantiation", &Instantiation_SemanticCheck, NULL, NULL, NULL);
    nkSparrowInnerInstantiationsSet =   registerNodeKind("spr.instantiationSet", &InstantiationsSet_SemanticCheck, NULL, NULL, NULL);

    firstSparrowNodeKind = nkSparrowModifiersNode;
}


Node* SprFrontend::mkModifiers(const Location& loc, Node* main, Node* mods)
{
    REQUIRE_NODE(loc, main);
    if ( !mods )
        return main;

    Node* res = createNode(nkSparrowModifiersNode);
    res->location = loc;
    res->children = { main, mods };
    return res;
}

Node* SprFrontend::mkSprCompilationUnit(const Location& loc, Node* package, Node* imports, Node* declarations)
{
    Node* res = createNode(nkSparrowDeclSprCompilationUnit);
    res->location = loc;
    res->children = { package, imports, declarations };
    ASSERT( !imports || imports->nodeKind == Feather::nkFeatherNodeList );
    ASSERT( !declarations || declarations->nodeKind == Feather::nkFeatherNodeList );
    return res;
}

Node* SprFrontend::mkSprUsing(const Location& loc, string alias, Node* usingNode, AccessType accessType)
{
    Node* res = createNode(nkSparrowDeclUsing);
    res->location = loc;
    res->children = { usingNode };
    if ( !alias.empty() )
        Feather::setName(res, move(alias));
    setAccessType(res, accessType);
    return res;
}

Node* SprFrontend::mkSprPackage(const Location& loc, string name, Node* children, AccessType accessType)
{
    Node* res = createNode(nkSparrowDeclPackage);
    res->location = loc;
    res->children = { children };
    Feather::setName(res, move(name));
    setAccessType(res, accessType);
    return res;
}

Node* SprFrontend::mkSprVariable(const Location& loc, string name, Node* typeNode, Node* init, AccessType accessType)
{
    Node* res = createNode(nkSparrowDeclSprVariable);
    res->location = loc;
    res->children = { typeNode, init };
    setName(res, move(name));
    setAccessType(res, accessType);
    return res;
}

Node* SprFrontend::mkSprVariable(const Location& loc, string name, TypeRef type, Node* init, AccessType accessType)
{
    Node* res = createNode(nkSparrowDeclSprVariable);
    res->location = loc;
    res->children = { nullptr, init };
    setName(res, move(name));
    setAccessType(res, accessType);
    setProperty(res, "spr.givenType", type);
    return res;
}

Node* SprFrontend::mkSprClass(const Location& loc, string name, Node* parameters, Node* baseClasses, Node* ifClause, Node* children, AccessType accessType)
{
    Node* res = createNode(nkSparrowDeclSprClass);
    res->location = loc;
    res->children = { parameters, baseClasses, children, ifClause };
    ASSERT( !parameters || parameters->nodeKind == nkFeatherNodeList );
    ASSERT( !baseClasses || baseClasses->nodeKind == nkFeatherNodeList );
    ASSERT( !children || children->nodeKind == nkFeatherNodeList );
    setName(res, move(name));
    setAccessType(res, accessType);
    return res;
}

Node* SprFrontend::mkSprConcept(const Location& loc, string name, string paramName, Node* baseConcept, Node* ifClause, AccessType accessType)
{
    Node* res = createNode(nkSparrowDeclSprConcept);
    res->location = loc;
    res->children = { baseConcept, ifClause, nullptr };
    setName(res, move(name));
    setAccessType(res, accessType);
    setProperty(res, "spr.paramName", move(paramName));
    return res;
}

Node* SprFrontend::mkSprFunction(const Location& loc, string name, Node* parameters, Node* returnType, Node* body, Node* ifClause, AccessType accessType)
{
    Node* res = createNode(nkSparrowDeclSprFunction);
    res->location = loc;
    res->children = { parameters, returnType, body, ifClause };
    ASSERT( !parameters || parameters->nodeKind == nkFeatherNodeList );
    setName(res, move(name));
    setAccessType(res, accessType);
    return res;
}

Node* SprFrontend::mkSprParameter(const Location& loc, string name, Node* typeNode, Node* init)
{
    Node* res = createNode(nkSparrowDeclSprParameter);
    res->location = loc;
    res->children = { typeNode, init };
    Feather::setName(res, move(name));
    return res;
}

Node* SprFrontend::mkSprParameter(const Location& loc, string name, TypeRef type, Node* init)
{
    Node* res = createNode(nkSparrowDeclSprParameter);
    res->location = loc;
    res->children = { nullptr, init };
    Feather::setName(res, move(name));
    setProperty(res, "spr.givenType", type);
    return res;
}

Node* SprFrontend::mkSprAutoParameter(const Location& loc, string name)
{
    Node* res = createNode(nkSparrowDeclSprParameter);
    res->location = loc;
    res->children = { mkIdentifier(loc, "AnyType"), nullptr };
    Feather::setName(res, move(name));
    return res;
}


Node* SprFrontend::mkGenericClass(Node* originalClass, Node* parameters, Node* ifClause)
{
    Node* res = createNode(nkSparrowDeclGenericClass);
    res->location = originalClass->location;
    res->children = { mkInstantiationsSet(originalClass, parameters->children, ifClause) };
    res->referredNodes = { originalClass };
    setName(res, getName(originalClass));
    setAccessType(res, publicAccess);
    setEvalMode(res, effectiveEvalMode(originalClass));

    // Semantic check the arguments
    for ( Node* param: parameters->children )
    {
        Nest::semanticCheck(param);
        if ( isConceptType(param->type) )
            REP_ERROR(param->location, "Cannot use auto or concept parameters for class generics");
    }
    return res;
}

Node* SprFrontend::mkGenericFunction(Node* originalFun, NodeVector params, NodeVector genericParams, Node* ifClause, Node* thisClass)
{
    Node* res = createNode(nkSparrowDeclGenericFunction);
    res->location = originalFun->location;
    res->children = { mkInstantiationsSet(originalFun, move(genericParams), ifClause) };
    res->referredNodes = { originalFun, mkNodeList(res->location, move(params)) };
    setName(res, getName(originalFun));
    setAccessType(res, publicAccess);
    setEvalMode(res, effectiveEvalMode(originalFun));
    return res;
}

Node* SprFrontend::mkLiteral(const Location& loc, string litType, string data)
{
    Node* res = createNode(nkSparrowExpLiteral);
    res->location = loc;
    setProperty(res, "spr.literalType", move(litType));
    setProperty(res, "spr.literalData", move(data));
    return res;
}

Node* SprFrontend::mkThisExp(const Location& loc)
{
    Node* res = createNode(nkSparrowExpThis);
    res->location = loc;
    return res;
}

Node* SprFrontend::mkIdentifier(const Location& loc, string id)
{
    Node* res = createNode(nkSparrowExpIdentifier);
    res->location = loc;
    setProperty(res, "name", move(id));
    setProperty(res, propAllowDeclExp, 0);
    return res;
}

Node* SprFrontend::mkCompoundExp(const Location& loc, Node* base, string id)
{
    Node* res = createNode(nkSparrowExpCompoundExp);
    res->location = loc;
    res->children = { base };
    setProperty(res, "name", move(id));
    setProperty(res, propAllowDeclExp, 0);
    return res;
}

Node* SprFrontend::mkFunApplication(const Location& loc, Node* base, Node* arguments)
{
    Node* res = createNode(nkSparrowExpFunApplication);
    res->location = loc;
    res->children = { base, arguments };
    ASSERT( !arguments || arguments->nodeKind == nkFeatherNodeList );
    if ( !arguments )
        res->children[1] = mkNodeList(loc, {});
    return res;
}
Node* SprFrontend::mkFunApplication(const Location& loc, Node* base, NodeVector arguments)
{
    Node* res = createNode(nkSparrowExpFunApplication);
    res->location = loc;
    res->children = { base, mkNodeList(loc, move(arguments)) };
    return res;
}

Node* SprFrontend::mkOperatorCall(const Location& loc, Node* arg1, string op, Node* arg2)
{
    Node* res = createNode(nkSparrowExpOperatorCall);
    res->location = loc;
    res->children = { arg1, arg2 };
    setProperty(res, "spr.operation", move(op));
    return res;
}

Node* SprFrontend::mkInfixOp(const Location& loc, string op, Node* arg1, Node* arg2)
{
    Node* res = createNode(nkSparrowExpInfixExp);
    res->location = loc;
    res->children = { arg1, arg2 };
    if ( op.empty() )
        REP_ERROR(res->location, "Operation name must have at least one character");
    setProperty(res, "spr.operation", move(op));
    return res;
}

Node* SprFrontend::mkLambdaExp(const Location& loc, Node* parameters, Node* returnType, Node* body, Node* bodyExp, Node* closureParams)
{
    if ( bodyExp )
    {
        ASSERT(!body);
        const Location& loc = bodyExp->location;
        body = Feather::mkLocalSpace(loc, { mkReturnStmt(loc, bodyExp) });
        if ( !returnType )
            returnType = mkFunApplication(loc, mkIdentifier(loc, "typeOf"), Feather::mkNodeList(loc, { bodyExp }));
    }
    Node* res = createNode(nkSparrowExpLambdaFunction);
    res->location = loc;
    res->children = {};
    res->referredNodes = { parameters, returnType, body, closureParams };
    ASSERT( !parameters || parameters->nodeKind == nkFeatherNodeList );
    ASSERT( !closureParams || closureParams->nodeKind == nkFeatherNodeList );
    return res;
}

Node* SprFrontend::mkConditionalExp(const Location& loc, Node* cond, Node* alt1, Node* alt2)
{
    Node* res = createNode(nkSparrowExpSprConditional);
    res->location = loc;
    res->children = { cond, alt1, alt2 };
    return res;
}

Node* SprFrontend::mkDeclExp(const Location& loc, NodeVector decls, Node* baseExp)
{
    Node* res = createNode(nkSparrowExpDeclExp);
    res->location = loc;
    res->children = { };
    res->referredNodes = move(decls);
    res->referredNodes.insert(res->referredNodes.begin(), baseExp);
    return res;
}

Node* SprFrontend::mkStarExp(const Location& loc, Node* base, const string& operName)
{
    if ( operName != "*" )
        REP_ERROR(loc, "Expected '*' in expression; found '%1%'") % operName;
    Node* res = createNode(nkSparrowExpStarExp);
    res->location = loc;
    res->children = { base };
    return res;
}


Node* SprFrontend::mkForStmt(const Location& loc, string name, Node* type, Node* range, Node* action)
{
    Node* res = createNode(nkSparrowStmtFor);
    res->location = loc;
    res->children = { range, action, type };
    setName(res, move(name));
    return res;
}

Node* SprFrontend::mkReturnStmt(const Location& loc, Node* exp)
{
    Node* res = createNode(nkSparrowStmtSprReturn);
    res->location = loc;
    res->children = { exp };
    return res;
}

Node* SprFrontend::mkInstantiation(const Location& loc, NodeVector boundValues, NodeVector boundVars)
{
    Node* res = createNode(nkSparrowInnerInstantiation);
    res->location = loc;
    res->children = { Feather::mkNodeList(loc, move(boundVars)) };
    res->referredNodes = move(boundValues);
    setProperty(res, "instIsValid", 0);
    setProperty(res, "instantiatedDecl", (Node*) nullptr);
    return res;
}

Node* SprFrontend::mkInstantiationsSet(Node* parentNode, NodeVector params, Node* ifClause)
{
    Location loc = parentNode->location;
    Node* res = createNode(nkSparrowInnerInstantiationsSet);
    res->location = loc;
    res->children = { ifClause, Feather::mkNodeList(loc, {}) };
    res->referredNodes = { parentNode, mkNodeList(loc, move(params)) };
    return res;
}
