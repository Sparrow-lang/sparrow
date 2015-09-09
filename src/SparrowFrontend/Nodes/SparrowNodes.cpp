#include <StdInc.h>
#include "SparrowNodes.h"
#include "SparrowNodesAccessors.h"
#include "Mods.h"

#include <Helpers/ForEachNodeInNodeList.h>
#include <Helpers/DeclsHelpers.h>
#include <Helpers/SprTypeTraits.h>
#include <Helpers/Convert.h>
#include <Helpers/CommonCode.h>


#include <Feather/Nodes/FeatherNodes.h>
#include <Feather/Util/Decl.h>
#include <Feather/Util/TypeTraits.h>
#include <Feather/Util/Context.h>

#include <Nest/Common/Diagnostic.hpp>
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
    Modifier* mod = nullptr;
    
    if ( modNode->nodeKind == nkSparrowExpIdentifier )
    {
        const string& name = Nest_getCheckPropertyString(modNode, "name");
        if ( name == "static" )
            mod = SprFe_getStaticMod();
        else if ( name == "ct" )
            mod = SprFe_getCtMod();
        else if ( name == "rt" )
            mod = SprFe_getRtMod();
        else if ( name == "rtct" )
            mod = SprFe_getRtCtMod();
        else if ( name == "autoCt" )
            mod = SprFe_getAutoCtMod();
        else if ( name == "ctGeneric" )
            mod = SprFe_getCtGenericMod();
        else if ( name == "convert" )
            mod = SprFe_getConvertMod();
        else if ( name == "noDefault" )
            mod = SprFe_getNoDefaultMod();
        else if ( name == "initCtor" )
            mod = SprFe_getInitCtorMod();
        else if ( name == "macro" )
            mod = SprFe_getMacroMod();
        else if ( name == "noInline" )
            mod = SprFe_getNoInlineMod();
    }
    else
    {
        // check for: native("name")
        if ( modNode->nodeKind == nkSparrowExpInfixExp )    // fun application
        {
            Node* fbase = at(modNode->children, 0);
            Node* fargs = at(modNode->children, 1);
            if ( fbase->nodeKind == nkSparrowExpIdentifier && Nest_getCheckPropertyString(fbase, "name") == "native" )
            {
                // one argument: a literal
                if ( fargs && fargs->nodeKind == Feather::nkFeatherNodeList && Nest_nodeArraySize(fargs->children) == 1
                && at(fargs->children, 0)->nodeKind == nkSparrowExpLiteral )
                {
                    Node* arg = at(fargs->children, 0);
                    if ( Literal_isString(arg) )
                    {
                        mod = SprFe_getNativeMod(Literal_getData(arg).c_str());
                    }
                }
            }
        }
    }
    
    // If we recognized a modifier, add it to the base node; otherwise raise an error
    if ( mod )
        Nest_addModifier(base, mod);
    else
        REP_ERROR(modNode->location, "Unknown modifier found: %1%") % modNode;
}
void ModifiersNode_SetContextForChildren(Node* node)
{
    Node* base = at(node->children, 0);
    Node* modifierNodes = at(node->children, 1);

    // Set the context of the modifiers
    if ( modifierNodes )
        Nest_setContext(modifierNodes, node->context);

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
        Nest_setContext(base, node->context);
}
TypeRef ModifiersNode_ComputeType(Node* node)
{
    Node* base = at(node->children, 0);
    return Nest_computeType(base);
}
Node* ModifiersNode_SemanticCheck(Node* node)
{
    return at(node->children, 0);
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
    ASSERT(Nest_nodeArraySize(node->children) == 3);
    Node* range = at(node->children, 0);
    Node* action = at(node->children, 1);
    Node* typeExpr = at(node->children, 2);

    ASSERT(range);
    CompilationContext* rangeContext = nodeEvalMode(node) == modeCt ? Nest_mkChildContext(node->context, modeCt) : node->context;
    if ( typeExpr )
        Nest_setContext(typeExpr, rangeContext);
    Nest_setContext(range, rangeContext);
    if ( action )
        Nest_setContext(action, node->context);
}

TypeRef For_ComputeType(Node* node)
{
    return getVoidType(node->context->evalMode);
}

Node* For_SemanticCheck(Node* node)
{
    ASSERT(Nest_nodeArraySize(node->children) == 3);
    Node* range = at(node->children, 0);
    Node* action = at(node->children, 1);
    Node* typeExpr = at(node->children, 2);

    bool ctFor = nodeEvalMode(node) == modeCt;

    if ( typeExpr && !Nest_semanticCheck(typeExpr) )
        return nullptr;
    if ( !Nest_semanticCheck(range) )
        return nullptr;

    const Location& loc = range->location;

    if ( ctFor && !isCt(range->type) )
        REP_ERROR_RET(nullptr, loc, "Range must be available at CT, for a CT for (range type: %1%)") % range->type;

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
    ASSERT(Nest_nodeArraySize(node->children) == 1);
    Node* exp = at(node->children, 0);

    // Get the parent function of this return
    Node* parentFun = getParentFun(node->context);
    if ( !parentFun )
        REP_ERROR_RET(nullptr, node->location, "Return found outside any function");

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
        if ( !Nest_semanticCheck(exp) )
            return nullptr;
        if ( !resType->hasStorage && exp->type == resType )
        {
            return mkNodeList(node->location, { exp, mkReturn(node->location) });
        }
        else
        {
            cvt = canConvert(exp, resType);
        }
        if ( !cvt )
            REP_ERROR_RET(nullptr, exp->location, "Cannot convert return expression (%1%) to %2%") % exp->type % resType;
    }
    else
    {
        if ( Function_resultType(parentFun)->typeKind != typeKindVoid )
            REP_ERROR_RET(nullptr, node->location, "You must return something in a function that has non-Void result type");
    }

    // Build the explanation of this node
    if ( resultParam )
    {
        // Create a ctor to construct the result parameter with the expression received
        const Location& l = resultParam->location;
        Node* thisArg = mkMemLoad(l, mkVarRef(l, resultParam));
        Nest_setContext(thisArg, node->context);
        Node* action = createCtorCall(l, node->context, thisArg, exp);
        if ( !action )
            REP_ERROR_RET(nullptr, exp->location, "Cannot construct return type object %1% from %2%") % exp->type % resType;

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
    nkSparrowModifiersNode =            Nest_registerNodeKind("spr.modifiers", &ModifiersNode_SemanticCheck, &ModifiersNode_ComputeType, &ModifiersNode_SetContextForChildren, NULL);

    nkSparrowDeclSprCompilationUnit =   Nest_registerNodeKind("spr.sprCompilationUnit", &SprCompilationUnit_SemanticCheck, &SprCompilationUnit_ComputeType, &SprCompilationUnit_SetContextForChildren, NULL);
    nkSparrowDeclPackage =              Nest_registerNodeKind("spr.package", &Package_SemanticCheck, &Package_ComputeType, &Package_SetContextForChildren, NULL);
    nkSparrowDeclSprClass =             Nest_registerNodeKind("spr.sprClass", &SprClass_SemanticCheck, &SprClass_ComputeType, &SprClass_SetContextForChildren, NULL);
    nkSparrowDeclSprFunction =          Nest_registerNodeKind("spr.sprFunction", &SprFunction_SemanticCheck, &SprFunction_ComputeType, &SprFunction_SetContextForChildren, NULL);
    nkSparrowDeclSprParameter =         Nest_registerNodeKind("spr.sprParameter", &SprParameter_SemanticCheck, &SprParameter_ComputeType, &SprParameter_SetContextForChildren, NULL);
    nkSparrowDeclSprVariable =          Nest_registerNodeKind("spr.sprVariable", &SprVariable_SemanticCheck, &SprVariable_ComputeType, &SprVariable_SetContextForChildren, NULL);
    nkSparrowDeclSprConcept =           Nest_registerNodeKind("spr.sprConcept", &SprConcept_SemanticCheck, NULL, &SprConcept_SetContextForChildren, NULL);
    nkSparrowDeclGenericClass =         Nest_registerNodeKind("spr.genericClass", &Generic_SemanticCheck, NULL, NULL, NULL);
    nkSparrowDeclGenericFunction =      Nest_registerNodeKind("spr.genericFunction", &Generic_SemanticCheck, NULL, NULL, NULL);
    nkSparrowDeclUsing =                Nest_registerNodeKind("spr.using", &Using_SemanticCheck, &Using_ComputeType, &Using_SetContextForChildren, NULL);

    nkSparrowExpLiteral =               Nest_registerNodeKind("spr.literal", &Literal_SemanticCheck, NULL, NULL, NULL);
    nkSparrowExpThis =                  Nest_registerNodeKind("spr.this", &This_SemanticCheck, NULL, NULL, NULL);
    nkSparrowExpIdentifier =            Nest_registerNodeKind("spr.identifier", &Identifier_SemanticCheck, NULL, NULL, NULL);
    nkSparrowExpCompoundExp =           Nest_registerNodeKind("spr.compoundExp", &CompoundExp_SemanticCheck, NULL, NULL, NULL);
    nkSparrowExpFunApplication =        Nest_registerNodeKind("spr.funApplication", &FunApplication_SemanticCheck, NULL, NULL, NULL);
    nkSparrowExpOperatorCall =          Nest_registerNodeKind("spr.operatorCall", &OperatorCall_SemanticCheck, NULL, NULL, NULL);
    nkSparrowExpInfixExp =              Nest_registerNodeKind("spr.infixExp", &InfixExp_SemanticCheck, NULL, NULL, NULL);
    nkSparrowExpLambdaFunction =        Nest_registerNodeKind("spr.lambdaFunction", &LambdaFunction_SemanticCheck, NULL, NULL, NULL);
    nkSparrowExpSprConditional =        Nest_registerNodeKind("spr.sprConditional", &SprConditional_SemanticCheck, NULL, NULL, NULL);
    nkSparrowExpDeclExp =               Nest_registerNodeKind("spr.declExp", &DeclExp_SemanticCheck, NULL, NULL, NULL);
    nkSparrowExpStarExp =               Nest_registerNodeKind("spr.starExp", &StarExp_SemanticCheck, NULL, NULL, NULL);
    
    nkSparrowStmtFor =                  Nest_registerNodeKind("spr.for", &For_SemanticCheck, &For_ComputeType, &For_SetContextForChildren, NULL);
    nkSparrowStmtSprReturn =            Nest_registerNodeKind("spr.return", &SprReturn_SemanticCheck, NULL, NULL, NULL);

    nkSparrowInnerInstantiation =       Nest_registerNodeKind("spr.instantiation", &Instantiation_SemanticCheck, NULL, NULL, NULL);
    nkSparrowInnerInstantiationsSet =   Nest_registerNodeKind("spr.instantiationSet", &InstantiationsSet_SemanticCheck, NULL, NULL, NULL);

    firstSparrowNodeKind = nkSparrowModifiersNode;
}


Node* SprFrontend::mkModifiers(const Location& loc, Node* main, Node* mods)
{
    CHECK(loc, main);
    if ( !mods )
        return main;

    Node* res = Nest_createNode(nkSparrowModifiersNode);
    res->location = loc;
    Nest_nodeSetChildren(res, fromIniList({ main, mods }));
    return res;
}

Node* SprFrontend::mkSprCompilationUnit(const Location& loc, Node* package, Node* imports, Node* declarations)
{
    Node* res = Nest_createNode(nkSparrowDeclSprCompilationUnit);
    res->location = loc;
    Nest_nodeSetChildren(res, fromIniList({ package, imports, declarations }));
    ASSERT( !imports || imports->nodeKind == Feather::nkFeatherNodeList );
    ASSERT( !declarations || declarations->nodeKind == Feather::nkFeatherNodeList );
    return res;
}

Node* SprFrontend::mkSprUsing(const Location& loc, string alias, Node* usingNode, AccessType accessType)
{
    Node* res = Nest_createNode(nkSparrowDeclUsing);
    res->location = loc;
    Nest_nodeSetChildren(res, fromIniList({ usingNode }));
    if ( !alias.empty() )
        Feather::setName(res, move(alias));
    setAccessType(res, accessType);
    return res;
}

Node* SprFrontend::mkSprPackage(const Location& loc, string name, Node* children, AccessType accessType)
{
    Node* res = Nest_createNode(nkSparrowDeclPackage);
    res->location = loc;
    Nest_nodeSetChildren(res, fromIniList({ children }));
    Feather::setName(res, move(name));
    setAccessType(res, accessType);
    return res;
}

Node* SprFrontend::mkSprVariable(const Location& loc, string name, Node* typeNode, Node* init, AccessType accessType)
{
    Node* res = Nest_createNode(nkSparrowDeclSprVariable);
    res->location = loc;
    Nest_nodeSetChildren(res, fromIniList({ typeNode, init }));
    setName(res, move(name));
    setAccessType(res, accessType);
    return res;
}

Node* SprFrontend::mkSprVariable(const Location& loc, string name, TypeRef type, Node* init, AccessType accessType)
{
    Node* res = Nest_createNode(nkSparrowDeclSprVariable);
    res->location = loc;
    Nest_nodeSetChildren(res, fromIniList({ nullptr, init }));
    setName(res, move(name));
    setAccessType(res, accessType);
    Nest_setProperty(res, "spr.givenType", type);
    return res;
}

Node* SprFrontend::mkSprClass(const Location& loc, string name, Node* parameters, Node* baseClasses, Node* ifClause, Node* children, AccessType accessType)
{
    Node* res = Nest_createNode(nkSparrowDeclSprClass);
    res->location = loc;
    Nest_nodeSetChildren(res, fromIniList({ parameters, baseClasses, children, ifClause }));
    ASSERT( !parameters || parameters->nodeKind == nkFeatherNodeList );
    ASSERT( !baseClasses || baseClasses->nodeKind == nkFeatherNodeList );
    ASSERT( !children || children->nodeKind == nkFeatherNodeList );
    setName(res, move(name));
    setAccessType(res, accessType);
    return res;
}

Node* SprFrontend::mkSprConcept(const Location& loc, string name, string paramName, Node* baseConcept, Node* ifClause, AccessType accessType)
{
    Node* res = Nest_createNode(nkSparrowDeclSprConcept);
    res->location = loc;
    Nest_nodeSetChildren(res, fromIniList({ baseConcept, ifClause, nullptr }));
    setName(res, move(name));
    setAccessType(res, accessType);
    Nest_setProperty(res, "spr.paramName", move(paramName));
    return res;
}

Node* SprFrontend::mkSprFunction(const Location& loc, string name, Node* parameters, Node* returnType, Node* body, Node* ifClause, AccessType accessType)
{
    Node* res = Nest_createNode(nkSparrowDeclSprFunction);
    res->location = loc;
    Nest_nodeSetChildren(res, fromIniList({ parameters, returnType, body, ifClause }));
    ASSERT( !parameters || parameters->nodeKind == nkFeatherNodeList );
    setName(res, move(name));
    setAccessType(res, accessType);
    return res;
}

Node* SprFrontend::mkSprParameter(const Location& loc, string name, Node* typeNode, Node* init)
{
    Node* res = Nest_createNode(nkSparrowDeclSprParameter);
    res->location = loc;
    Nest_nodeSetChildren(res, fromIniList({ typeNode, init }));
    Feather::setName(res, move(name));
    return res;
}

Node* SprFrontend::mkSprParameter(const Location& loc, string name, TypeRef type, Node* init)
{
    Node* res = Nest_createNode(nkSparrowDeclSprParameter);
    res->location = loc;
    Nest_nodeSetChildren(res, fromIniList({ nullptr, init }));
    Feather::setName(res, move(name));
    Nest_setProperty(res, "spr.givenType", type);
    return res;
}

Node* SprFrontend::mkSprAutoParameter(const Location& loc, string name)
{
    Node* res = Nest_createNode(nkSparrowDeclSprParameter);
    res->location = loc;
    Nest_nodeSetChildren(res, fromIniList({ mkIdentifier(loc, "AnyType"), nullptr }));
    Feather::setName(res, move(name));
    return res;
}


Node* SprFrontend::mkGenericClass(Node* originalClass, Node* parameters, Node* ifClause)
{
    Node* res = Nest_createNode(nkSparrowDeclGenericClass);
    res->location = originalClass->location;
    Nest_nodeSetChildren(res, fromIniList({ mkInstantiationsSet(originalClass, toVec(parameters->children), ifClause) }));
    Nest_nodeSetReferredNodes(res, fromIniList({ originalClass }));
    setName(res, getName(originalClass));
    setAccessType(res, publicAccess);
    setEvalMode(res, effectiveEvalMode(originalClass));

    // Semantic check the arguments
    for ( Node* param: parameters->children )
    {
        if ( !Nest_semanticCheck(param) )
            return nullptr;
        if ( isConceptType(param->type) )
            REP_ERROR_RET(nullptr, param->location, "Cannot use auto or concept parameters for class generics");
    }
    return res;
}

Node* SprFrontend::mkGenericFunction(Node* originalFun, NodeVector params, NodeVector genericParams, Node* ifClause, Node* thisClass)
{
    Node* res = Nest_createNode(nkSparrowDeclGenericFunction);
    res->location = originalFun->location;
    Nest_nodeSetChildren(res, fromIniList({ mkInstantiationsSet(originalFun, move(genericParams), ifClause) }));
    Nest_nodeSetReferredNodes(res, fromIniList({ originalFun, mkNodeList(res->location, move(params)) }));
    setName(res, getName(originalFun));
    setAccessType(res, publicAccess);
    setEvalMode(res, effectiveEvalMode(originalFun));
    return res;
}

Node* SprFrontend::mkLiteral(const Location& loc, string litType, string data)
{
    Node* res = Nest_createNode(nkSparrowExpLiteral);
    res->location = loc;
    Nest_setProperty(res, "spr.literalType", move(litType));
    Nest_setProperty(res, "spr.literalData", move(data));
    return res;
}

Node* SprFrontend::mkThisExp(const Location& loc)
{
    Node* res = Nest_createNode(nkSparrowExpThis);
    res->location = loc;
    return res;
}

Node* SprFrontend::mkIdentifier(const Location& loc, string id)
{
    Node* res = Nest_createNode(nkSparrowExpIdentifier);
    res->location = loc;
    Nest_setProperty(res, "name", move(id));
    Nest_setProperty(res, propAllowDeclExp, 0);
    return res;
}

Node* SprFrontend::mkCompoundExp(const Location& loc, Node* base, string id)
{
    Node* res = Nest_createNode(nkSparrowExpCompoundExp);
    res->location = loc;
    Nest_nodeSetChildren(res, fromIniList({ base }));
    Nest_setProperty(res, "name", move(id));
    Nest_setProperty(res, propAllowDeclExp, 0);
    return res;
}

Node* SprFrontend::mkFunApplication(const Location& loc, Node* base, Node* arguments)
{
    Node* res = Nest_createNode(nkSparrowExpFunApplication);
    res->location = loc;
    Nest_nodeSetChildren(res, fromIniList({ base, arguments }));
    ASSERT( !arguments || arguments->nodeKind == nkFeatherNodeList );
    if ( !arguments )
        at(res->children, 1) = mkNodeList(loc, {});
    return res;
}
Node* SprFrontend::mkFunApplication(const Location& loc, Node* base, NodeVector arguments)
{
    Node* res = Nest_createNode(nkSparrowExpFunApplication);
    res->location = loc;
    Nest_nodeSetChildren(res, fromIniList({ base, mkNodeList(loc, move(arguments)) }));
    return res;
}

Node* SprFrontend::mkOperatorCall(const Location& loc, Node* arg1, string op, Node* arg2)
{
    Node* res = Nest_createNode(nkSparrowExpOperatorCall);
    res->location = loc;
    Nest_nodeSetChildren(res, fromIniList({ arg1, arg2 }));
    Nest_setProperty(res, "spr.operation", move(op));
    return res;
}

Node* SprFrontend::mkInfixOp(const Location& loc, string op, Node* arg1, Node* arg2)
{
    Node* res = Nest_createNode(nkSparrowExpInfixExp);
    res->location = loc;
    Nest_nodeSetChildren(res, fromIniList({ arg1, arg2 }));
    if ( op.empty() )
        REP_ERROR_RET(nullptr, res->location, "Operation name must have at least one character");
    Nest_setProperty(res, "spr.operation", move(op));
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
    Node* res = Nest_createNode(nkSparrowExpLambdaFunction);
    res->location = loc;
    Nest_nodeSetChildren(res, fromIniList({}));
    Nest_nodeSetReferredNodes(res, fromIniList({ parameters, returnType, body, closureParams }));
    ASSERT( !parameters || parameters->nodeKind == nkFeatherNodeList );
    ASSERT( !closureParams || closureParams->nodeKind == nkFeatherNodeList );
    return res;
}

Node* SprFrontend::mkConditionalExp(const Location& loc, Node* cond, Node* alt1, Node* alt2)
{
    Node* res = Nest_createNode(nkSparrowExpSprConditional);
    res->location = loc;
    Nest_nodeSetChildren(res, fromIniList({ cond, alt1, alt2 }));
    return res;
}

Node* SprFrontend::mkDeclExp(const Location& loc, NodeVector decls, Node* baseExp)
{
    Node* res = Nest_createNode(nkSparrowExpDeclExp);
    res->location = loc;
    Nest_nodeSetChildren(res, fromIniList({ }));
    Nest_appendNodeToArray(&res->referredNodes, baseExp);
    Nest_appendNodesToArray(&res->referredNodes, all(decls));
    return res;
}

Node* SprFrontend::mkStarExp(const Location& loc, Node* base, const string& operName)
{
    if ( operName != "*" )
        REP_ERROR_RET(nullptr, loc, "Expected '*' in expression; found '%1%'") % operName;
    Node* res = Nest_createNode(nkSparrowExpStarExp);
    res->location = loc;
    Nest_nodeSetChildren(res, fromIniList({ base }));
    return res;
}


Node* SprFrontend::mkForStmt(const Location& loc, string name, Node* type, Node* range, Node* action)
{
    Node* res = Nest_createNode(nkSparrowStmtFor);
    res->location = loc;
    Nest_nodeSetChildren(res, fromIniList({ range, action, type }));
    setName(res, move(name));
    return res;
}

Node* SprFrontend::mkReturnStmt(const Location& loc, Node* exp)
{
    Node* res = Nest_createNode(nkSparrowStmtSprReturn);
    res->location = loc;
    Nest_nodeSetChildren(res, fromIniList({ exp }));
    return res;
}

Node* SprFrontend::mkInstantiation(const Location& loc, NodeVector boundValues, NodeVector boundVars)
{
    Node* res = Nest_createNode(nkSparrowInnerInstantiation);
    res->location = loc;
    Nest_nodeSetChildren(res, fromIniList({ Feather::mkNodeList(loc, move(boundVars)) }));
    Nest_appendNodesToArray(&res->referredNodes, all(boundValues));
    Nest_setProperty(res, "instIsValid", 0);
    Nest_setProperty(res, "instantiatedDecl", (Node*) nullptr);
    return res;
}

Node* SprFrontend::mkInstantiationsSet(Node* parentNode, NodeVector params, Node* ifClause)
{
    Location loc = parentNode->location;
    Node* res = Nest_createNode(nkSparrowInnerInstantiationsSet);
    res->location = loc;
    Nest_nodeSetChildren(res, fromIniList({ ifClause, Feather::mkNodeList(loc, {}) }));
    Nest_nodeSetReferredNodes(res, fromIniList({ parentNode, mkNodeList(loc, move(params)) }));
    return res;
}
