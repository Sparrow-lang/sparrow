#include <StdInc.h>
#include "SparrowNodes.h"
#include "SparrowNodesAccessors.h"
#include "Mods.h"
#include "SprDebug.h"

#include <Helpers/ForEachNodeInNodeList.h>
#include <Helpers/DeclsHelpers.h>
#include <Helpers/SprTypeTraits.h>
#include <Helpers/Convert.h>
#include <Helpers/CommonCode.h>

#include "Feather/Api/Feather.h"
#include "Feather/Utils/FeatherUtils.hpp"
#include "Feather/Utils/cppif/FeatherTypes.hpp"

#include "Nest/Utils/Diagnostic.hpp"
#include "Nest/Api/NodeKindRegistrar.h"

using namespace SprFrontend;
using namespace Feather;
using namespace Nest;

#define UNUSED(x) (void)(x)

////////////////////////////////////////////////////////////////////////////////
// ModifiersNode
//

/// Apply the given modifier node to the given base node
/// Recurse down if the base node is a node list or a modifiers node
void applyModifier(Node* base, Node* modNode) {
    Nest_Modifier* mod = nullptr;

    if (!base)
        return;

    // Recurse down if the base if is a node list or a modifiers node
    if (base->nodeKind == nkFeatherNodeList) {
        for (Node* n : base->children)
            applyModifier(n, modNode);
        return;
    } else if (base->nodeKind == nkSparrowModifiersNode) {
        applyModifier(at(base->children, 0), modNode);
        return;
    }

    // Interpret the modifier expression
    if (modNode->nodeKind == nkSparrowExpIdentifier) {
        Nest_StringRef name = Nest_getCheckPropertyString(modNode, "name");
        if (name == "public")
            mod = SprFe_getPublicMod();
        else if (name == "protected")
            mod = SprFe_getProtectedMod();
        else if (name == "private")
            mod = SprFe_getPrivateMod();
        else if (name == "ct")
            mod = SprFe_getCtMod();
        else if (name == "rt")
            mod = SprFe_getRtMod();
        else if (name == "autoCt")
            mod = SprFe_getAutoCtMod();
        else if (name == "ctGeneric")
            mod = SprFe_getCtGenericMod();
        else if (name == "convert")
            mod = SprFe_getConvertMod();
        else if (name == "noDefault")
            mod = SprFe_getNoDefaultMod();
        else if (name == "initCtor")
            mod = SprFe_getInitCtorMod();
        else if (name == "macro")
            mod = SprFe_getMacroMod();
        else if (name == "noInline")
            mod = SprFe_getNoInlineMod();
    } else {
        // check for: native("name")
        if (modNode->nodeKind == nkSparrowExpFunApplication)
        {
            Node* fbase = at(modNode->children, 0);
            Node* fargs = at(modNode->children, 1);
            if (fbase->nodeKind == nkSparrowExpIdentifier &&
                    Nest_getCheckPropertyString(fbase, "name") == "native") {
                // one argument: a literal
                if (fargs && fargs->nodeKind == nkFeatherNodeList &&
                        Nest_nodeArraySize(fargs->children) == 1 &&
                        at(fargs->children, 0)->nodeKind == nkSparrowExpLiteral) {
                    Node* arg = at(fargs->children, 0);
                    if (Literal_isString(arg)) {
                        mod = SprFe_getNativeMod(Literal_getData(arg));
                    }
                }
            }
        }
    }

    // If we recognized a modifier, add it to the base node; otherwise raise an error
    if (mod)
        Nest_addModifier(base, mod);
    else
        REP_ERROR(modNode->location, "Unknown modifier found: %1%") % modNode;
}
void ModifiersNode_SetContextForChildren(Node* node) {
    Node* base = at(node->children, 0);
    Node* modifierNodes = at(node->children, 1);

    // Set the context of the modifiers
    if (modifierNodes)
        Nest_setContext(modifierNodes, node->context);

    // Interpret the modifiers here - as much as possible
    if (modifierNodes) {
        if (modifierNodes->nodeKind == nkFeatherNodeList) {
            // Add the modifiers to the base node
            forEachNodeInNodeList(
                    modifierNodes, [&](Node* modNode) { applyModifier(base, modNode); });
        } else {
            applyModifier(base, modifierNodes);
        }
    }

    // Set the context for the base
    if (base)
        Nest_setContext(base, node->context);
}
TypeRef ModifiersNode_ComputeType(Node* node) {
    Node* base = at(node->children, 0);
    return Nest_computeType(base);
}
Node* ModifiersNode_SemanticCheck(Node* node) { return at(node->children, 0); }

////////////////////////////////////////////////////////////////////////////////
// Instantiation
//

Node* Instantiation_SemanticCheck(Node* node) { return Feather_mkNop(node->location); }

////////////////////////////////////////////////////////////////////////////////
// InstantiationSet
//

Node* InstantiationsSet_SemanticCheck(Node* node) { return Feather_mkNop(node->location); }

////////////////////////////////////////////////////////////////////////////////
// For
//

void For_SetContextForChildren(Node* node) {
    ASSERT(Nest_nodeArraySize(node->children) == 3);
    Node* range = at(node->children, 0);
    Node* action = at(node->children, 1);
    Node* typeExpr = at(node->children, 2);

    ASSERT(range);
    CompilationContext* rangeContext = Feather_nodeEvalMode(node) == modeCt
                                               ? Nest_mkChildContext(node->context, modeCt)
                                               : node->context;
    if (typeExpr)
        Nest_setContext(typeExpr, rangeContext);
    Nest_setContext(range, rangeContext);
    if (action)
        Nest_setContext(action, node->context);
}

TypeRef For_ComputeType(Node* node) { return Feather_getVoidType(node->context->evalMode); }

Node* For_SemanticCheck(Node* node) {
    ASSERT(Nest_nodeArraySize(node->children) == 3);
    Node* range = at(node->children, 0);
    Node* action = at(node->children, 1);
    Node* typeExpr = at(node->children, 2);

    bool ctFor = Feather_nodeEvalMode(node) == modeCt;

    if (typeExpr && !Nest_semanticCheck(typeExpr))
        return nullptr;
    if (!Nest_semanticCheck(range))
        return nullptr;

    const Location& loc = range->location;

    if (ctFor && range->type->mode != modeCt)
        REP_ERROR_RET(
                nullptr, loc, "Range must be available at CT, for a CT for (range type: %1%)") %
                range->type;

    // Expand the for statement of the form
    //      for ( <name>: <type> = <range> ) action;
    //
    // into:
    //      var $rangeVar: Range = <range>;
    //      while ( ! ($rangeVar isEmpty) ; $rangeVar popFront )
    //      {
    //          var <name>: <type> = $rangeVar front;
    //          action;
    //      }
    //
    // if <type> is not present, we will use '$rangeType.RetType'

    // Variable to hold the range - initialize it with the range node
    Node* rangeVar =
            mkSprVariable(loc, StringRef("$rangeVar"), mkIdentifier(loc, StringRef("Range")), range);
    if (ctFor)
        Feather_setEvalMode(rangeVar, modeCt);
    Node* rangeVarRef = mkIdentifier(loc, StringRef("$rangeVar"));

    // while condition
    Node* base1 = mkOperatorCall(loc, rangeVarRef, StringRef("isEmpty"), nullptr);
    Node* whileCond = mkOperatorCall(loc, nullptr, StringRef("!"), base1);

    // while step
    Node* whileStep = mkOperatorCall(loc, rangeVarRef, StringRef("popFront"), nullptr);

    // while body
    Node* whileBody = nullptr;
    if (action) {
        if (!typeExpr)
            typeExpr = mkCompoundExp(loc, rangeVarRef, StringRef("RetType"));

        // the iteration variable
        Node* init = mkOperatorCall(loc, rangeVarRef, StringRef("front"), nullptr);

        Node* iterVar = mkSprVariable(node->location, Feather_getName(node), typeExpr, init);
        if (ctFor)
            Feather_setEvalMode(iterVar, modeCt);

        whileBody = Feather_mkLocalSpace(action->location, fromIniList({iterVar, action}));
    }

    Node* whileStmt = Feather_mkWhile(loc, whileCond, whileBody, whileStep);
    if (ctFor)
        Feather_setEvalMode(whileStmt, modeCt);

    return Feather_mkLocalSpace(node->location, fromIniList({rangeVar, whileStmt}));
}

////////////////////////////////////////////////////////////////////////////////
// SprReturn
//

Node* SprReturn_SemanticCheck(Node* node) {
    ASSERT(Nest_nodeArraySize(node->children) == 1);
    Node* exp = at(node->children, 0);

    // Get the parent function of this return
    Node* parentFun = Feather_getParentFun(node->context);
    if (!parentFun)
        REP_ERROR_RET(nullptr, node->location, "Return found outside any function");

    // Compute the result type of the function
    TypeRef resType = nullptr;
    Node* resultParam = getResultParam(parentFun);
    if (resultParam) // Does this function have a result param?
    {
        resType = removeRef(TypeWithStorage(resultParam->type));
        ASSERT(!Feather_Function_resultType(parentFun)
                        ->hasStorage); // The function should have void result
    } else {
        resType = Feather_Function_resultType(parentFun);
    }
    ASSERT(resType);

    // Check match between return expression and function result type
    ConversionResult cvt = convNone;
    if (exp) {
        if (!Nest_semanticCheck(exp))
            return nullptr;
        if (!resType->hasStorage && exp->type == resType) {
            return Feather_mkNodeList(
                    node->location, fromIniList({exp, Feather_mkReturn(node->location, nullptr)}));
        } else {
            cvt = g_ConvertService->checkConversion(exp, resType);
        }
        if (!cvt)
            REP_ERROR_RET(nullptr, exp->location, "Cannot convert return expression (%1%) to %2%") %
                    exp->type % resType;
    } else {
        if (resultParam || Feather_Function_resultType(parentFun)->typeKind != typeKindVoid)
            REP_ERROR_RET(nullptr, node->location,
                    "You must return something in a function that has non-Void result type");
    }

    // Build the explanation of this node
    if (resultParam) {
        // Create a ctor to construct the result parameter with the expression received
        const Location& l = resultParam->location;
        Node* thisArg = Feather_mkMemLoad(l, Feather_mkVarRef(l, resultParam));
        Nest_setContext(thisArg, node->context);
        Node* action = createCtorCall(l, node->context, thisArg, exp);
        if (!action)
            REP_ERROR_RET(
                    nullptr, exp->location, "Cannot construct return type object %1% from %2%") %
                    exp->type % resType;

        return Feather_mkNodeList(
                node->location, fromIniList({action, Feather_mkReturn(node->location, nullptr)}));
    } else {
        exp = exp ? cvt.apply(node->context, exp) : nullptr;
        return Feather_mkReturn(node->location, exp);
    }
}

////////////////////////////////////////////////////////////////////////////////
// Function forwards - implemented in SparrowNodes_Decl.cpp
//

void Module_SetContextForChildren(Node* node);
Node* Module_SemanticCheck(Node* node);

void ImportName_SetContextForChildren(Node* node);
Node* ImportName_SemanticCheck(Node* node);
const char* ImportName_toString(Node* node);

void Package_SetContextForChildren(Node* node);
TypeRef Package_ComputeType(Node* node);
Node* Package_SemanticCheck(Node* node);

void SprDatatype_SetContextForChildren(Node* node);
TypeRef SprDatatype_ComputeType(Node* node);
Node* SprDatatype_SemanticCheck(Node* node);

void SprField_SetContextForChildren(Node* node);
TypeRef SprField_ComputeType(Node* node);
Node* SprField_SemanticCheck(Node* node);

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
Node* Identifier_SemanticCheck(Node* node);
Node* CompoundExp_SemanticCheck(Node* node);
const char* CompoundExp_toString(Node* node);
Node* FunApplication_SemanticCheck(Node* node);
Node* OperatorCall_SemanticCheck(Node* node);
Node* InfixExp_SemanticCheck(Node* node);
Node* LambdaFunction_SemanticCheck(Node* node);
Node* SprConditional_SemanticCheck(Node* node);
Node* DeclExp_SemanticCheck(Node* node);
Node* StarExp_SemanticCheck(Node* node);
Node* ModuleRef_SemanticCheck(Node* node);

////////////////////////////////////////////////////////////////////////////////
// Node kind variables
//

int SprFrontend::firstSparrowNodeKind = 0;

int SprFrontend::nkSparrowModifiersNode = 0;

int SprFrontend::nkSparrowDeclModule = 0;
int SprFrontend::nkSparrowDeclImportName = 0;
int SprFrontend::nkSparrowDeclPackage = 0;
int SprFrontend::nkSparrowDeclSprDatatype = 0;
int SprFrontend::nkSparrowDeclSprFunction = 0;
int SprFrontend::nkSparrowDeclSprParameter = 0;
int SprFrontend::nkSparrowDeclSprField = 0;
int SprFrontend::nkSparrowDeclSprVariable = 0;
int SprFrontend::nkSparrowDeclSprConcept = 0;
int SprFrontend::nkSparrowDeclGenericPackage = 0;
int SprFrontend::nkSparrowDeclGenericClass = 0;
int SprFrontend::nkSparrowDeclGenericFunction = 0;
int SprFrontend::nkSparrowDeclUsing = 0;

int SprFrontend::nkSparrowExpLiteral = 0;
int SprFrontend::nkSparrowExpIdentifier = 0;
int SprFrontend::nkSparrowExpCompoundExp = 0;
int SprFrontend::nkSparrowExpFunApplication = 0;
int SprFrontend::nkSparrowExpOperatorCall = 0;
int SprFrontend::nkSparrowExpInfixExp = 0;
int SprFrontend::nkSparrowExpLambdaFunction = 0;
int SprFrontend::nkSparrowExpSprConditional = 0;
int SprFrontend::nkSparrowExpDeclExp = 0;
int SprFrontend::nkSparrowExpStarExp = 0;
int SprFrontend::nkSparrowExpModuleRef = 0;

int SprFrontend::nkSparrowStmtFor = 0;
int SprFrontend::nkSparrowStmtSprReturn = 0;

int SprFrontend::nkSparrowInnerInstantiation = 0;
int SprFrontend::nkSparrowInnerInstantiationsSet = 0;

////////////////////////////////////////////////////////////////////////////////
// Functions from the header
//

void SprFrontend::initSparrowNodeKinds() {
    nkSparrowModifiersNode = Nest_registerNodeKind("spr.modifiers", &ModifiersNode_SemanticCheck,
            &ModifiersNode_ComputeType, &ModifiersNode_SetContextForChildren, nullptr);

    nkSparrowDeclModule = Nest_registerNodeKind(
            "spr.module", &Module_SemanticCheck, nullptr, &Module_SetContextForChildren, nullptr);
    nkSparrowDeclImportName = Nest_registerNodeKind("spr.importName", &ImportName_SemanticCheck,
            nullptr, &ImportName_SetContextForChildren, &ImportName_toString);
    nkSparrowDeclPackage = Nest_registerNodeKind("spr.package", &Package_SemanticCheck,
            &Package_ComputeType, &Package_SetContextForChildren, nullptr);
    nkSparrowDeclSprDatatype = Nest_registerNodeKind("spr.SprDatatype", &SprDatatype_SemanticCheck,
            &SprDatatype_ComputeType, &SprDatatype_SetContextForChildren, nullptr);
    nkSparrowDeclSprField = Nest_registerNodeKind("spr.sprField", &SprField_SemanticCheck,
            &SprField_ComputeType, &SprField_SetContextForChildren, nullptr);
    nkSparrowDeclSprFunction = Nest_registerNodeKind("spr.sprFunction", &SprFunction_SemanticCheck,
            &SprFunction_ComputeType, &SprFunction_SetContextForChildren, nullptr);
    nkSparrowDeclSprParameter =
            Nest_registerNodeKind("spr.sprParameter", &SprParameter_SemanticCheck,
                    &SprParameter_ComputeType, &SprParameter_SetContextForChildren, nullptr);
    nkSparrowDeclSprVariable = Nest_registerNodeKind("spr.sprVariable", &SprVariable_SemanticCheck,
            &SprVariable_ComputeType, &SprVariable_SetContextForChildren, nullptr);
    nkSparrowDeclSprConcept = Nest_registerNodeKind("spr.sprConcept", &SprConcept_SemanticCheck,
            nullptr, &SprConcept_SetContextForChildren, nullptr);
    nkSparrowDeclGenericPackage = Nest_registerNodeKind(
            "spr.genericPackage", &Generic_SemanticCheck, nullptr, nullptr, nullptr);
    nkSparrowDeclGenericClass = Nest_registerNodeKind(
            "spr.genericClass", &Generic_SemanticCheck, nullptr, nullptr, nullptr);
    nkSparrowDeclGenericFunction = Nest_registerNodeKind(
            "spr.genericFunction", &Generic_SemanticCheck, nullptr, nullptr, nullptr);
    nkSparrowDeclUsing = Nest_registerNodeKind("spr.using", &Using_SemanticCheck,
            &Using_ComputeType, &Using_SetContextForChildren, nullptr);

    nkSparrowExpLiteral =
            Nest_registerNodeKind("spr.literal", &Literal_SemanticCheck, nullptr, nullptr, nullptr);
    nkSparrowExpIdentifier = Nest_registerNodeKind(
            "spr.identifier", &Identifier_SemanticCheck, nullptr, nullptr, nullptr);
    nkSparrowExpCompoundExp = Nest_registerNodeKind(
            "spr.compoundExp", &CompoundExp_SemanticCheck, nullptr, nullptr, &CompoundExp_toString);
    nkSparrowExpFunApplication = Nest_registerNodeKind(
            "spr.funApplication", &FunApplication_SemanticCheck, nullptr, nullptr, nullptr);
    nkSparrowExpOperatorCall = Nest_registerNodeKind(
            "spr.operatorCall", &OperatorCall_SemanticCheck, nullptr, nullptr, nullptr);
    nkSparrowExpInfixExp = Nest_registerNodeKind(
            "spr.infixExp", &InfixExp_SemanticCheck, nullptr, nullptr, nullptr);
    nkSparrowExpLambdaFunction = Nest_registerNodeKind(
            "spr.lambdaFunction", &LambdaFunction_SemanticCheck, nullptr, nullptr, nullptr);
    nkSparrowExpSprConditional = Nest_registerNodeKind(
            "spr.sprConditional", &SprConditional_SemanticCheck, nullptr, nullptr, nullptr);
    nkSparrowExpDeclExp =
            Nest_registerNodeKind("spr.declExp", &DeclExp_SemanticCheck, nullptr, nullptr, nullptr);
    nkSparrowExpStarExp =
            Nest_registerNodeKind("spr.starExp", &StarExp_SemanticCheck, nullptr, nullptr, nullptr);
    nkSparrowExpModuleRef = Nest_registerNodeKind(
            "spr.moduleRef", &ModuleRef_SemanticCheck, nullptr, nullptr, nullptr);

    nkSparrowStmtFor = Nest_registerNodeKind(
            "spr.for", &For_SemanticCheck, &For_ComputeType, &For_SetContextForChildren, nullptr);
    nkSparrowStmtSprReturn = Nest_registerNodeKind(
            "spr.return", &SprReturn_SemanticCheck, nullptr, nullptr, nullptr);

    nkSparrowInnerInstantiation = Nest_registerNodeKind(
            "spr.instantiation", &Instantiation_SemanticCheck, nullptr, nullptr, nullptr);
    nkSparrowInnerInstantiationsSet = Nest_registerNodeKind(
            "spr.instantiationSet", &InstantiationsSet_SemanticCheck, nullptr, nullptr, nullptr);

    firstSparrowNodeKind = nkSparrowModifiersNode;
}

Node* SprFrontend::mkModifiers(const Location& loc, Node* main, Node* mods) {
    CHECK(loc, main);
    if (!mods)
        return main;

    Node* res = Nest_createNode(nkSparrowModifiersNode);
    res->location = loc;
    Nest_nodeSetChildren(res, fromIniList({main, mods}));
    return res;
}

Node* SprFrontend::mkModule(const Location& loc, Node* moduleName, Node* declarations) {
    Node* res = Nest_createNode(nkSparrowDeclModule);
    res->location = loc;
    Nest_nodeSetChildren(res, fromIniList({moduleName, declarations}));
    ASSERT(!declarations || declarations->nodeKind == nkFeatherNodeList);
    return res;
}

Node* SprFrontend::mkImportName(const Location& loc, Node* moduleName, Node* importedDeclNames,
        bool equals, Nest_StringRef alias) {
    Node* res = Nest_createNode(nkSparrowDeclImportName);
    res->location = loc;
    Nest_nodeSetChildren(res, fromIniList({moduleName, importedDeclNames}));
    ASSERT(!importedDeclNames || importedDeclNames->nodeKind == nkFeatherNodeList);
    if (equals)
        Feather_setName(res, alias);
    return res;
}

Node* SprFrontend::mkSprUsing(const Location& loc, Nest_StringRef alias, Node* usingNode) {
    Node* res = Nest_createNode(nkSparrowDeclUsing);
    res->location = loc;
    Nest_nodeSetChildren(res, fromIniList({usingNode}));
    if (StringRef(alias))
        Feather_setName(res, alias);
    deduceAccessType(res);
    return res;
}

Node* SprFrontend::mkSprPackage(
        const Location& loc, Nest_StringRef name, Node* children, Node* params, Node* ifClause) {
    Node* res = Nest_createNode(nkSparrowDeclPackage);
    res->location = loc;
    Nest_nodeSetChildren(res, fromIniList({children, params, ifClause}));
    Feather_setName(res, name);
    deduceAccessType(res);
    return res;
}

Node* SprFrontend::mkSprVariable(const Location& loc, Nest_StringRef name, Node* typeNode, Node* init) {
    Node* res = Nest_createNode(nkSparrowDeclSprVariable);
    res->location = loc;
    Nest_nodeSetChildren(res, fromIniList({typeNode, init}));
    Feather_setName(res, name);
    deduceAccessType(res);
    return res;
}

Node* SprFrontend::mkSprVariable(const Location& loc, Nest_StringRef name, TypeRef type, Node* init) {
    Node* res = Nest_createNode(nkSparrowDeclSprVariable);
    res->location = loc;
    Nest_nodeSetChildren(res, fromIniList({nullptr, init}));
    Feather_setName(res, name);
    deduceAccessType(res);
    Nest_setPropertyType(res, "spr.givenType", type);
    return res;
}

Node* SprFrontend::mkSprDatatype(const Location& loc, Nest_StringRef name, Node* parameters,
        Node* underlyingData, Node* ifClause, Node* children) {
    Node* res = Nest_createNode(nkSparrowDeclSprDatatype);
    res->location = loc;
    if (underlyingData) {
        ASSERT(!children);
        Node* innerVar = mkSprField(loc, StringRef("data"), underlyingData, nullptr);
        children = Feather_addToNodeList(children, innerVar);
        Nest_setPropertyInt(res, propGenerateInitCtor, 1);
    }
    Nest_nodeSetChildren(res, fromIniList({parameters, children, ifClause}));
    ASSERT(!parameters || parameters->nodeKind == nkFeatherNodeList);
    ASSERT(!children || children->nodeKind == nkFeatherNodeList);
    Feather_setName(res, name);
    deduceAccessType(res);
    return res;
}

Node* SprFrontend::mkSprField(const Location& loc, Nest_StringRef name, Node* typeNode, Node* init) {
    Node* res = Nest_createNode(nkSparrowDeclSprField);
    res->location = loc;
    Nest_nodeSetChildren(res, fromIniList({typeNode, init}));
    Feather_setName(res, name);
    return res;
}

Node* SprFrontend::mkSprConcept(const Location& loc, Nest_StringRef name, Nest_StringRef paramName,
        Node* baseConcept, Node* ifClause) {
    Node* res = Nest_createNode(nkSparrowDeclSprConcept);
    res->location = loc;
    Nest_nodeSetChildren(res, fromIniList({baseConcept, ifClause, nullptr}));
    Feather_setName(res, name);
    deduceAccessType(res);
    Nest_setPropertyString(res, "spr.paramName", paramName);
    return res;
}

Node* SprFrontend::mkSprFunction(const Location& loc, Nest_StringRef name, Node* parameters,
        Node* returnType, Node* body, Node* ifClause) {
    Node* res = Nest_createNode(nkSparrowDeclSprFunction);
    res->location = loc;
    Nest_nodeSetChildren(res, fromIniList({parameters, returnType, body, ifClause}));
    ASSERT(!parameters || parameters->nodeKind == nkFeatherNodeList);
    Feather_setName(res, name);
    deduceAccessType(res);
    return res;
}

Node* SprFrontend::mkSprParameter(const Location& loc, Nest_StringRef name, Node* typeNode, Node* init) {
    Node* res = Nest_createNode(nkSparrowDeclSprParameter);
    res->location = loc;
    Nest_nodeSetChildren(res, fromIniList({typeNode, init}));
    Feather_setName(res, name);
    return res;
}

Node* SprFrontend::mkSprParameter(const Location& loc, Nest_StringRef name, TypeRef type, Node* init) {
    Node* res = Nest_createNode(nkSparrowDeclSprParameter);
    res->location = loc;
    Nest_nodeSetChildren(res, fromIniList({nullptr, init}));
    Feather_setName(res, name);
    Nest_setPropertyType(res, "spr.givenType", type);
    return res;
}

Node* SprFrontend::mkSprAutoParameter(const Location& loc, Nest_StringRef name) {
    Node* res = Nest_createNode(nkSparrowDeclSprParameter);
    res->location = loc;
    Nest_nodeSetChildren(res, fromIniList({mkIdentifier(loc, StringRef("AnyType")), nullptr}));
    Feather_setName(res, name);
    return res;
}

Node* SprFrontend::mkGenericPackage(Node* originalPackage, Node* parameters, Node* ifClause) {
    Node* res = Nest_createNode(nkSparrowDeclGenericPackage);
    res->location = originalPackage->location;
    Nest_nodeSetChildren(res, fromIniList({mkInstantiationsSet(
                                      originalPackage, all(parameters->children), ifClause)}));
    Nest_nodeSetReferredNodes(res, fromIniList({originalPackage}));
    Feather_setName(res, Feather_getName(originalPackage));
    copyAccessType(res, originalPackage);
    Feather_setEvalMode(res, Feather_effectiveEvalMode(originalPackage));

    Nest_appendNodeToArray(&res->additionalNodes, originalPackage);

    // Semantic check the arguments
    for (Node* param : parameters->children) {
        if (!Nest_semanticCheck(param))
            return nullptr;
        if (isConceptType(param->type))
            REP_ERROR_RET(nullptr, param->location,
                    "Cannot use auto or concept parameters for package generics");
    }
    return res;
}

Node* SprFrontend::mkGenericClass(Node* originalClass, Node* parameters, Node* ifClause) {
    Node* res = Nest_createNode(nkSparrowDeclGenericClass);
    res->location = originalClass->location;
    Nest_nodeSetChildren(res,
            fromIniList({mkInstantiationsSet(originalClass, all(parameters->children), ifClause)}));
    Nest_nodeSetReferredNodes(res, fromIniList({originalClass}));
    Feather_setName(res, Feather_getName(originalClass));
    copyAccessType(res, originalClass);
    Feather_setEvalMode(res, Feather_effectiveEvalMode(originalClass));

    Nest_appendNodeToArray(&res->additionalNodes, originalClass);

    // Semantic check the arguments
    for (Node* param : parameters->children) {
        if (!Nest_semanticCheck(param))
            return nullptr;
        if (isConceptType(param->type))
            REP_ERROR_RET(nullptr, param->location,
                    "Cannot use auto or concept parameters for class generics");
    }
    return res;
}

Node* SprFrontend::mkGenericFunction(Node* originalFun, Nest_NodeRange params, Nest_NodeRange genericParams,
        Node* ifClause, Node* thisClass) {
    Node* res = Nest_createNode(nkSparrowDeclGenericFunction);
    res->location = originalFun->location;
    Nest_nodeSetChildren(
            res, fromIniList({mkInstantiationsSet(originalFun, move(genericParams), ifClause)}));
    Nest_nodeSetReferredNodes(
            res, fromIniList({originalFun, Feather_mkNodeList(res->location, params)}));
    Feather_setName(res, Feather_getName(originalFun));
    copyAccessType(res, originalFun);
    Feather_setEvalMode(res, Feather_effectiveEvalMode(originalFun));
    return res;
}

Node* SprFrontend::mkLiteral(const Location& loc, Nest_StringRef litType, Nest_StringRef data) {
    Node* res = Nest_createNode(nkSparrowExpLiteral);
    res->location = loc;
    Nest_setPropertyString(res, "spr.literalType", litType);
    Nest_setPropertyString(res, "spr.literalData", data);
    return res;
}

Node* SprFrontend::mkIdentifier(const Location& loc, Nest_StringRef id) {
    Node* res = Nest_createNode(nkSparrowExpIdentifier);
    res->location = loc;
    Nest_setPropertyString(res, "name", id);
    return res;
}

Node* SprFrontend::mkCompoundExp(const Location& loc, Node* base, Nest_StringRef id) {
    Node* res = Nest_createNode(nkSparrowExpCompoundExp);
    res->location = loc;
    Nest_nodeSetChildren(res, fromIniList({base}));
    Nest_setPropertyString(res, "name", id);
    return res;
}

Node* SprFrontend::mkFunApplication(const Location& loc, Node* base, Node* arguments) {
    Node* res = Nest_createNode(nkSparrowExpFunApplication);
    res->location = loc;
    Nest_nodeSetChildren(res, fromIniList({base, arguments}));
    ASSERT(!arguments || arguments->nodeKind == nkFeatherNodeList);
    if (!arguments)
        at(res->children, 1) = Feather_mkNodeList(loc, {});
    return res;
}
Node* SprFrontend::mkFunApplication(const Location& loc, Node* base, Nest_NodeRange arguments) {
    Node* res = Nest_createNode(nkSparrowExpFunApplication);
    res->location = loc;
    Nest_nodeSetChildren(res, fromIniList({base, Feather_mkNodeList(loc, arguments)}));
    return res;
}

Node* SprFrontend::mkOperatorCall(const Location& loc, Node* arg1, Nest_StringRef op, Node* arg2) {
    Node* res = Nest_createNode(nkSparrowExpOperatorCall);
    res->location = loc;
    Nest_nodeSetChildren(res, fromIniList({arg1, arg2}));
    Nest_setPropertyString(res, "spr.operation", op);
    return res;
}

Node* SprFrontend::mkInfixOp(const Location& loc, Nest_StringRef op, Node* arg1, Node* arg2) {
    Node* res = Nest_createNode(nkSparrowExpInfixExp);
    res->location = loc;
    Nest_nodeSetChildren(res, fromIniList({arg1, arg2}));
    if (StringRef(op).empty())
        REP_ERROR_RET(nullptr, res->location, "Operation name must have at least one character");
    Nest_setPropertyString(res, "spr.operation", op);
    return res;
}

Node* SprFrontend::mkLambdaExp(const Location& loc, Node* parameters, Node* returnType, Node* body,
        Node* bodyExp, Node* closureParams) {
    if (bodyExp) {
        ASSERT(!body);
        const Location& loc = bodyExp->location;
        body = Feather_mkLocalSpace(loc, fromIniList({mkReturnStmt(loc, bodyExp)}));
        if (!returnType)
            returnType = mkFunApplication(loc, mkIdentifier(loc, StringRef("typeOf")),
                    Feather_mkNodeList(loc, fromIniList({bodyExp})));
    }
    Node* res = Nest_createNode(nkSparrowExpLambdaFunction);
    res->location = loc;
    Nest_nodeSetChildren(res, fromIniList({}));
    Nest_nodeSetReferredNodes(res, fromIniList({parameters, returnType, body, closureParams}));
    ASSERT(!parameters || parameters->nodeKind == nkFeatherNodeList);
    ASSERT(!closureParams || closureParams->nodeKind == nkFeatherNodeList);
    return res;
}

Node* SprFrontend::mkConditionalExp(const Location& loc, Node* cond, Node* alt1, Node* alt2) {
    Node* res = Nest_createNode(nkSparrowExpSprConditional);
    res->location = loc;
    Nest_nodeSetChildren(res, fromIniList({cond, alt1, alt2}));
    return res;
}

Node* SprFrontend::mkDeclExp(const Location& loc, Nest_NodeRange decls, Node* baseExp) {
    Node* res = Nest_createNode(nkSparrowExpDeclExp);
    res->location = loc;
    Nest_nodeSetChildren(res, fromIniList({}));
    Nest_appendNodeToArray(&res->referredNodes, baseExp);
    Nest_appendNodesToArray(&res->referredNodes, decls);
    return res;
}

Node* SprFrontend::mkStarExp(const Location& loc, Node* base, Nest_StringRef operName) {
    if (operName != "*")
        REP_ERROR_RET(nullptr, loc, "Expected '*' in expression; found '%1%'") % operName;
    Node* res = Nest_createNode(nkSparrowExpStarExp);
    res->location = loc;
    Nest_nodeSetChildren(res, fromIniList({base}));
    return res;
}

Node* SprFrontend::mkModuleRef(const Location& loc, Node* module) {
    Node* res = Nest_createNode(nkSparrowExpModuleRef);
    res->location = loc;
    Nest_nodeSetChildren(res, fromIniList({}));
    Nest_nodeSetReferredNodes(res, fromIniList({module}));
    return res;
}

Node* SprFrontend::mkForStmt(
        const Location& loc, Nest_StringRef name, Node* type, Node* range, Node* action) {
    Node* res = Nest_createNode(nkSparrowStmtFor);
    res->location = loc;
    Nest_nodeSetChildren(res, fromIniList({range, action, type}));
    Feather_setName(res, name);
    return res;
}

Node* SprFrontend::mkReturnStmt(const Location& loc, Node* exp) {
    Node* res = Nest_createNode(nkSparrowStmtSprReturn);
    res->location = loc;
    Nest_nodeSetChildren(res, fromIniList({exp}));
    return res;
}

Node* SprFrontend::mkInstantiation(
        const Location& loc, Nest_NodeRange boundValues, Nest_NodeRange boundVars) {
    Node* res = Nest_createNode(nkSparrowInnerInstantiation);
    res->location = loc;
    Nest_nodeSetChildren(res, fromIniList({Feather_mkNodeListVoid(loc, boundVars)}));
    Nest_appendNodesToArray(&res->referredNodes, boundValues);
    Nest_setPropertyInt(res, "instIsValid", 0);
    Nest_setPropertyInt(res, "instIsEvaluated", 0);
    Nest_setPropertyNode(res, "instantiatedDecl", nullptr);
    return res;
}

Node* SprFrontend::mkInstantiationsSet(Node* parentNode, Nest_NodeRange params, Node* ifClause) {
    Location loc = parentNode->location;
    Node* res = Nest_createNode(nkSparrowInnerInstantiationsSet);
    res->location = loc;
    Nest_nodeSetChildren(res, fromIniList({ifClause, Feather_mkNodeList(loc, {})}));
    Nest_nodeSetReferredNodes(res, fromIniList({parentNode, Feather_mkNodeList(loc, params)}));
    return res;
}
