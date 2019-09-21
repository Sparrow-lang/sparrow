#include <StdInc.h>
#include "IntMods.h"
#include "Mods.h"
#include "SprDebug.h"

#include <Nodes/Builder.h>
#include <Helpers/SprTypeTraits.h>
#include <Helpers/DeclsHelpers.h>
#include <Helpers/StdDef.h>
#include "Feather/Api/Feather.h"
#include "Feather/Utils/FeatherUtils.hpp"
#include "Feather/Utils/cppif/FeatherTypes.hpp"
#include "Feather/Utils/cppif/FeatherNodes.hpp"

#include "Nest/Api/Modifier.h"
#include "Nest/Api/SourceCode.h"
#include "Nest/Utils/Diagnostic.hpp"

using namespace SprFrontend;
using namespace std;

namespace {
const int generatedOverloadPrio = -100;
const int defaultOverloadPrio = 0;

//! Checks if the given field indicates a true reference type (without cat)
bool fieldIsRef(Node* field) {
    auto t = Feather::removeCategoryIfPresent(Type(field->type));
    return t.numReferences() > 0;
}

// Add to a local space an operator call
void addOperatorCall(Node* dest, bool reverse, Node* operand1, const string& op, Node* operand2) {
    Node* call = mkOperatorCall(dest->location, operand1, StringRef(op), operand2);
    if (!reverse)
        Nest_appendNodeToArray(&dest->children, call);
    else
        Nest_insertNodeIntoArray(&dest->children, 0, call);
}

// Add an associated function with the given body and given arguments, near the parent class
// If autoCt==true, and mode==modeRt, we will also add the autoCt modifier
// This is useful for things like the '==' operator
Node* addAssociatedFun(Node* parent, const string& name, Node* body,
        vector<pair<Type, string>> params, int overloadPrio, Node* resClass = nullptr,
        EvalMode mode = modeUnspecified, bool autoCt = false) {
    Location loc = parent->location;
    loc.end = loc.start;

    CompilationContext* ctx = classContext(parent);

    // Construct the parameters list, return type node
    NodeVector sprParams;
    sprParams.reserve(params.size());
    for (auto param : params) {
        auto p = mkSprParameter(loc, StringRef(param.second), param.first);
        if (param.first.mode() == modeCt)
            Nest_setPropertyInt(p, propNoAutoConst, 1);
        sprParams.push_back(p);
    }
    Node* parameters = sprParams.empty() ? nullptr : Feather_mkNodeList(loc, all(sprParams));
    Node* ret =
            resClass ? createTypeNode(ctx, loc, Feather::DataType::get(resClass, modeRt)) : nullptr;

    // Add the function in the context of the parent
    Node* f = mkSprFunction(loc, StringRef(name), parameters, ret, body);
    Nest_setPropertyInt(f, propNoDefault, 1);
    Nest_setPropertyInt(f, "spr.accessType", (int)protectedAccess);
    Nest_setPropertyExplInt(f, propOverloadPrio, generatedOverloadPrio);
    if (mode == modeUnspecified)
        mode = Feather_effectiveEvalMode(parent);
    Feather_setEvalMode(f, mode);
    if (mode == modeRt && autoCt)
        Nest_addModifier(f, SprFe_getAutoCtMod());
    Nest_setContext(f, ctx);
    if (!Nest_computeType(f))
        return nullptr;
    Nest_queueSemanticCheck(f);
    Nest_appendNodeToArray(&parent->additionalNodes, f);
    return f;
}

/// Generate an associated function with the given name, by calling 'op' for the base classes and
/// fields
Node* generateAssociatedFun(
        Node* parent, const string& name, const string& op, Type otherParam, bool reverse = false) {
    Location loc = parent->location;
    loc.end = loc.start;
    Node* cls = Nest_explanation(parent);
    cls = cls && cls->nodeKind == nkFeatherDeclClass ? cls : nullptr;
    ASSERT(cls);

    Node* thisRef = mkIdentifier(loc, StringRef("this"));

    Node* otherRef = nullptr;
    if (otherParam) {
        otherRef = mkIdentifier(loc, StringRef("other"));
    }

    // Construct the body
    Node* body = Feather_mkLocalSpace(loc, {});
    for (Node* field : cls->children) {
        // Left-hand side: field-ref to the current field
        Node* lhs = Feather_mkFieldRef(loc, thisRef, field);

        // Right-hand side
        Node* rhs =
                otherParam
                        ? mkCompoundExp(loc, otherRef, Feather_getName(field))
                        : (name == "ctor" ? Nest_getPropertyDefaultNode(field, propVarInit, nullptr)
                                          : nullptr);

        string oper = op;
        if (fieldIsRef(field)) {
            if (op == "=" || op == "ctor") {
                oper = ":="; // Transform into ref assignment
                if (!rhs)
                    rhs = buildNullLiteral(loc);
            } else if (op == "dtor")
                continue; // Nothing to destruct on references
        }
        addOperatorCall(body, reverse, lhs, oper, rhs);
    }

    vector<pair<Type, string>> params;
    params.reserve(2);
    params.emplace_back(Feather::MutableType::get(Feather::DataType(cls->type)), string("this"));
    if (otherParam)
        params.emplace_back(otherParam, string("other"));

    return addAssociatedFun(parent, name, body, params, generatedOverloadPrio, nullptr);
}

/// Generate an init ctor, that initializes all the members with data received as arguments
/// Returns true if all fields were initialized with default values, and no params are needed.
bool generateInitCtor(Node* parent) {
    vector<pair<Type, string>> params;
    params.emplace_back(Feather::addRef(Feather::DataType(parent->type)), "this");

    Location loc = parent->location;
    loc.end = loc.start;
    Node* cls = Nest_explanation(parent);
    cls = cls && cls->nodeKind == nkFeatherDeclClass ? cls : nullptr;
    ASSERT(cls);

    Node* thisRef = mkIdentifier(loc, StringRef("this"));

    // Construct the body
    Node* body = Feather_mkLocalSpace(loc, {});
    for (Node* field : cls->children) {
        Type t = field->type;

        // Do we have an initialization specified in the data struct?
        Node* init = Nest_getPropertyDefaultNode(field, propVarInit, nullptr);
        if (!init) {
            // Add a parameter for the base
            string paramName = "f" + StringRef(Feather_getName(field)).toStd();
            params.emplace_back(t, paramName);
            init = mkIdentifier(loc, StringRef(paramName));
            if (t.numReferences() > 0)
                init = Feather_mkMemLoad(loc, init);
        }

        // Left-hand side: field-ref to the current field
        Node* lhs = Feather_mkFieldRef(loc, Feather_mkMemLoad(loc, thisRef), field);

        string oper = fieldIsRef(field) ? ":=" : "ctor";
        addOperatorCall(body, false, lhs, oper, init);
    }

    (void)addAssociatedFun(parent, "ctor", body, params, defaultOverloadPrio);
    return params.size() == 1;
}

/// Generate the equality check method for the given class
Node* generateEqualityCheckFun(Node* parent) {
    Location loc = parent->location;
    loc.end = loc.start;
    Node* cls = Nest_explanation(parent);
    cls = cls && cls->nodeKind == nkFeatherDeclClass ? cls : nullptr;
    ASSERT(cls);

    // Construct the equality check expression
    Node* exp = nullptr;
    for (Node* field : cls->children) {
        Node* fieldRef = Feather_mkFieldRef(
                loc, mkIdentifier(loc, StringRef("this")), field);
        Node* otherFieldRef = Feather_mkFieldRef(
                loc, mkIdentifier(loc, StringRef("other")), field);

        const char* op = fieldIsRef(field) ? "===" : "==";
        Node* curExp = mkOperatorCall(loc, fieldRef, StringRef(op), otherFieldRef);
        if (!exp)
            exp = curExp;
        else
            exp = mkOperatorCall(loc, exp, StringRef("&&"), curExp);
    }
    if (!exp)
        exp = buildBoolLiteral(loc, true);

    Node* body = Feather_mkLocalSpace(loc, {});
    Nest_appendNodeToArray(&body->children, mkReturnStmt(loc, exp));

    vector<pair<Type, string>> params;
    params.reserve(2);
    auto t = Feather::DataType::get(cls);
    auto tConst = Feather::ConstType::get(t);
    params.emplace_back(tConst, string("this"));
    params.emplace_back(tConst, string("other"));
    Node* res = addAssociatedFun(parent, "==", body, params, generatedOverloadPrio, StdDef::clsBool,
            Feather_effectiveEvalMode(parent), true);
    return res;
}

//! Gets the type for the 'this' parameter of the given function
Node* getThisTypeForFun(Node* fun) {
    // Get the class of the 'this' param
    int thisIdx = getThisParamIdx(fun);
    if (thisIdx != 0)
        REP_INTERNAL(fun->location, "'this' parameter should be the first parameter");
    Node* parameters = at(fun->children, 0);
    Node* thisParam = at(parameters->children, thisIdx);
    CHECK(fun->location, thisParam);
    if (!Nest_computeType(thisParam))
        return nullptr;
    ASSERT(thisParam->type);
    Node* cls = thisParam->type->referredNode;
    ASSERT(cls);
    return cls;
}

/// Checks if the given field can have a constructor/destructor call.
/// We check if the given field is of a data-like type
bool canHaveCtorDtor(Node* field) {
    CHECK(field->location, field->type);
    return Feather::isDataLikeType(field->type);
}

/// Search the given body for a constructor with the given properties.
///
/// This can search for constructors of given classes, constructors called on this, or called for
/// the given field.
///
/// It will search only the instructions directly inside the given local space, or in a child local
/// space It will not search inside conditionals, or other instructions
///
/// Can also be applied to finding destructors
bool hasCtorCall(Node* inSpace, bool checkThis, Node* forField, StringRef funName = "ctor") {
    // Check all the items in the local space
    for (Node* n : inSpace->children) {
        if (!Nest_computeType(n))
            continue;
        n = Nest_explanation(n);
        if (!n)
            continue;

        // Check inner local spaces
        if (n->nodeKind == nkFeatherLocalSpace) {
            if (hasCtorCall(n, checkThis, forField))
                return true;
            continue;
        }

        // We consider function calls for our checks
        if (n->nodeKind != nkFeatherExpFunCall)
            continue;
        Node* funDecl = at(n->referredNodes, 0);
        if (Feather_getName(funDecl) != funName)
            continue;
        if (Nest_nodeArraySize(n->children) == 0)
            continue;
        Node* thisArg = at(n->children, 0);
        if (!Nest_semanticCheck(thisArg))
            continue;
        thisArg = Nest_explanation(thisArg);
        if (!thisArg)
            continue;

        // Check for this to be passed as argument
        if (checkThis) {
            // If we have a MemLoad, just ignore it
            while (thisArg && thisArg->nodeKind == nkFeatherExpMemLoad)
                thisArg = Nest_explanation(at(thisArg->children, 0));

            if (!thisArg || thisArg->nodeKind != nkFeatherExpVarRef ||
                    Feather_getName(at(thisArg->referredNodes, 0)) != StringRef("this"))
                continue;
        }

        // If a field is given, check that the this argument is a reference to our field
        if (forField) {
            // If we have a Bitcast, just ignore it
            if (thisArg->nodeKind == nkFeatherExpBitcast)
                thisArg = Nest_explanation(at(thisArg->children, 0));

            if (!thisArg || thisArg->nodeKind != nkFeatherExpFieldRef ||
                    at(thisArg->referredNodes, 0) != forField)
                continue;
        }

        return true;
    }
    return false;
}
} // namespace

void _IntModClassMembers_afterComputeType(Nest_Modifier*, Node* node) {
    // Check to apply only to classes
    if (node->nodeKind != nkSparrowDeclSprDatatype)
        REP_INTERNAL(node->location, "IntModClassMembers modifier can be applied only to classes");
    Node* datatype = node;
    if (!datatype->type)
        REP_INTERNAL(
                node->location, "Type was not computed for %1% when applying IntModClassMembers") %
                Feather_getName(node);

    auto structDecl = NodeHandle(node).explanation().kindCast<Feather::StructDecl>();
    ASSERT(structDecl);
    Type paramType = Feather::DataType::get(structDecl, modeRt);

    // Initialization ctor
    bool skipDefaultCtor = false;
    if (Nest_hasProperty(datatype, propGenerateInitCtor))
        skipDefaultCtor = generateInitCtor(datatype);

    // Auto-generated functions
    if (!skipDefaultCtor)
        generateAssociatedFun(datatype, "ctor", "ctor", nullptr);
    generateAssociatedFun(datatype, "ctor", "ctor", paramType);
    if (Feather_effectiveEvalMode(structDecl) == modeRt) {
        Type paramCt = Feather::DataType::get(structDecl, modeCt);
        generateAssociatedFun(datatype, "ctorFromCt", "ctor", paramCt);
    }
    generateAssociatedFun(datatype, "dtor", "dtor", nullptr, true);
    generateAssociatedFun(datatype, "=", "=", paramType);
    generateEqualityCheckFun(datatype);
}

void IntModCtorMembers_beforeSemanticCheck(Nest_Modifier*, Node* fun) {
    // Check to apply only to non-static constructors
    if (fun->nodeKind != nkSparrowDeclSprFunction || Feather_getName(fun) != StringRef("ctor"))
        REP_INTERNAL(
                fun->location, "IntModCtorMembers modifier can be applied only to constructors");
    if (!funHasThisParameters(fun))
        REP_INTERNAL(fun->location, "IntModCtorMembers cannot be applied to static constructors");

    // If we have a body, make sure it's a local space
    Node* body = at(fun->children, 2);
    if (!body)
        return; // nothing to do
    if (body->nodeKind != nkFeatherLocalSpace)
        REP_INTERNAL(fun->location,
                "Constructor body is not a local space (needed by IntModCtorMembers)");

    // Get the class
    Node* cls = getThisTypeForFun(fun);
    if (!cls)
        return;

    // If we are calling other constructor of this class, don't add any initialization
    if (hasCtorCall(body, true, nullptr))
        return;

    // Generate the ctor calls in the order of the fields; add them to the body of the constructor
    const Location& loc = body->location;
    for (int i = Nest_nodeArraySize(cls->children) - 1; i >= 0; --i) {
        Node* field = at(cls->children, i);
        if (!canHaveCtorDtor(field))
            continue;

        if (!hasCtorCall(body, false, field)) {
            Node* base = mkCompoundExp(
                    loc, mkIdentifier(loc, StringRef("this")), Feather_getName(field));
            Node* call = nullptr;
            if (!fieldIsRef(field)) {
                call = mkOperatorCall(loc, base, StringRef("ctor"), nullptr);
            } else {
                call = mkOperatorCall(loc, base, StringRef(":="), buildNullLiteral(loc));
            }
            Nest_setContext(call, Nest_childrenContext(body));
            Nest_insertNodeIntoArray(&body->children, 0, call);
        }
    }
}

void IntModDtorMembers_beforeSemanticCheck(Nest_Modifier*, Node* fun) {
    /// Check to apply only to non-static destructors
    if (fun->nodeKind != nkSparrowDeclSprFunction || Feather_getName(fun) != StringRef("dtor"))
        REP_INTERNAL(
                fun->location, "IntModDtorMembers modifier can be applied only to destructors");
    if (!funHasThisParameters(fun))
        REP_INTERNAL(fun->location, "IntModDtorMembers cannot be applied to static destructors");

    // If we have a body, make sure it's a local space
    Node* body = at(fun->children, 2);
    if (!body)
        return; // nothing to do
    if (body->nodeKind != nkFeatherLocalSpace)
        REP_INTERNAL(fun->location,
                "Destructor body is not a local space (needed by IntModDtorMembers)");

    Node* cls = getThisTypeForFun(fun);
    if (!cls)
        return;

    // Generate the dtor calls in reverse order of the fields; add them to the body of the
    // destructor
    CompilationContext* context = Nest_childrenContext(body);
    const Location& loc = body->location;
    for (int i = Nest_nodeArraySize(cls->children) - 1; i >= 0; --i) {
        Node* field = at(cls->children, i);

        if (!canHaveCtorDtor(field))
            continue;

        if (hasCtorCall(body, false, field, "dtor"))
            continue;
        if (!fieldIsRef(field)) {
            Node* base = mkCompoundExp(
                    loc, mkIdentifier(loc, StringRef("this")), Feather_getName(field));
            Node* call = mkOperatorCall(loc, base, StringRef("dtor"), nullptr);
            Nest_setContext(call, context);
            Nest_appendNodeToArray(&body->children, call);
        }
    }
}

Nest_Modifier _classMembersIntMod = {
        modTypeAfterComputeType, &_IntModClassMembers_afterComputeType};
Nest_Modifier _ctorMembersIntMod = {
        modTypeBeforeSemanticCheck, &IntModCtorMembers_beforeSemanticCheck};
Nest_Modifier _dtorMembersIntMod = {
        modTypeBeforeSemanticCheck, &IntModDtorMembers_beforeSemanticCheck};

Nest_Modifier* SprFe_getClassMembersIntMod() { return &_classMembersIntMod; }
Nest_Modifier* SprFe_getCtorMembersIntMod() { return &_ctorMembersIntMod; }
Nest_Modifier* SprFe_getDtorMembersIntMod() { return &_dtorMembersIntMod; }
