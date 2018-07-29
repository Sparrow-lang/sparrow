#include "Feather/src/StdInc.h"
#include "Feather/Utils/FeatherUtils.hpp"

#include "Feather/Api/Feather.h"
#include "Nest/Api/Node.h"
#include "Nest/Api/CompilationContext.h"
#include "Nest/Api/SymTab.h"
#include "Nest/Utils/cppif/NodeHandle.hpp"
#include "Nest/Utils/cppif/NodeUtils.hpp"
#include "Nest/Utils/cppif/StringRef.hpp"
#include "Nest/Utils/Diagnostic.hpp"

using Nest::NodeHandle;

/// Tests if the given node is a declaration (a node that will expand to a Feather declaration)
bool _isDecl(Node* node) {
    switch (Nest_explanation(node)->nodeKind - Feather_getFirstFeatherNodeKind()) {
    case nkRelFeatherDeclFunction:
    case nkRelFeatherDeclClass:
    case nkRelFeatherDeclVar:
        return true;
    default:
        return false;
    }
}

int Feather_isCt(Node* node) { return node->type && node->type->mode == modeCt; }

int _isTestable(TypeRef type) {
    // If not Testable, check at least that is some kind of boolean
    if (!type || !type->hasStorage)
        return false;
    StringRef nativeName = Feather_nativeName(type);
    return nativeName && (nativeName == "i1" || nativeName == "u1");
}

int Feather_isTestable(Node* node) { return _isTestable(node->type); }

int Feather_isBasicNumericType(TypeRef type) {
    if (!type || !type->hasStorage)
        return false;
    StringRef nativeName = Feather_nativeName(type);
    return !nativeName.empty() &&
           (nativeName == "i1" || nativeName == "u1" || nativeName == "i8" || nativeName == "u8" ||
                   nativeName == "i16" || nativeName == "u16" || nativeName == "i32" ||
                   nativeName == "u32" || nativeName == "i64" || nativeName == "u64" ||
                   nativeName == "float" || nativeName == "double");
}

TypeRef Feather_checkChangeTypeMode(TypeRef type, EvalMode mode, Location loc) {
    ASSERT(type);
    if (mode == type->mode)
        return type;

    TypeRef resType = Nest_changeTypeMode(type, mode);
    if (!resType)
        REP_INTERNAL(loc, "Don't know how to change eval mode of type %1%") % type;
    ASSERT(resType);

    if (mode == modeCt && resType->mode != modeCt)
        REP_ERROR_RET(nullptr, loc, "Type '%1%' cannot be used at compile-time") % type;

    return resType;
}

TypeRef Feather_addRef(TypeRef type) {
    ASSERT(type);
    if (!type->hasStorage)
        REP_INTERNAL(Location(), "Invalid type given when adding reference (%1%)") % type;
    if (type->typeKind != typeKindData && type->typeKind != typeKindLValue)
        REP_INTERNAL(Location(), "Invalid type given when adding reference (%1%)") % type;
    return Feather_getDataType(type->referredNode, type->numReferences + 1, type->mode);
}

TypeRef Feather_removeRef(TypeRef type) {
    ASSERT(type);
    if (!type->hasStorage || type->numReferences < 1)
        REP_INTERNAL(Location(), "Invalid type given when removing reference (%1%)") % type;
    if (type->typeKind != typeKindData && type->typeKind != typeKindLValue)
        REP_INTERNAL(Location(), "Invalid type given when removing reference (%1%)") % type;
    return Feather_getDataType(type->referredNode, type->numReferences - 1, type->mode);
}

TypeRef Feather_removeAllRef(TypeRef type) {
    ASSERT(type);
    if (!type->hasStorage)
        REP_INTERNAL(Location(), "Invalid type given when removing reference (%1%)") % type;
    if (type->typeKind != typeKindData && type->typeKind != typeKindLValue)
        REP_INTERNAL(Location(), "Invalid type given when removing all references (%1%)") % type;
    return Feather_getDataType(type->referredNode, 0, type->mode);
}

TypeRef Feather_removeLValue(TypeRef type) {
    ASSERT(type);
    if (type->typeKind != typeKindLValue)
        REP_INTERNAL(Location(), "Expected l-value type; got %1%") % type;
    return Feather_getDataType(type->referredNode, type->numReferences - 1, type->mode);
}

TypeRef Feather_removeLValueIfPresent(TypeRef type) {
    ASSERT(type);
    if (type->typeKind != typeKindLValue)
        return type;
    return Feather_getDataType(type->referredNode, type->numReferences - 1, type->mode);
}

TypeRef Feather_lvalueToRef(TypeRef type) {
    ASSERT(type);
    if (type->typeKind != typeKindLValue)
        REP_INTERNAL(Location(), "Expected l-value type; got %1%") % type;
    return Feather_getDataType(type->referredNode, type->numReferences, type->mode);
}

TypeRef Feather_lvalueToRefIfPresent(TypeRef type) {
    ASSERT(type);
    if (type->typeKind != typeKindLValue)
        return type;
    return Feather_getDataType(type->referredNode, type->numReferences, type->mode);
}

Node* Feather_classForType(TypeRef t) { return t->hasStorage ? t->referredNode : nullptr; }

int Feather_isSameTypeIgnoreMode(TypeRef t1, TypeRef t2) {
    ASSERT(t1);
    ASSERT(t2);
    if (t1 == t2)
        return true;
    if (t1->typeKind != t2->typeKind || t1->mode == t2->mode)
        return false;
    TypeRef t = Feather_checkChangeTypeMode(t1, t2->mode, NOLOC);
    return t == t2;
}

EvalMode Feather_combineMode(EvalMode mode1, EvalMode mode2) {
    if (mode1 == modeCt || mode2 == modeCt)
        return modeCt;
    if (mode1 == modeUnspecified)
        return mode2;
    return mode1;
}

EvalMode Feather_combineModeBottom(EvalMode mode1, EvalMode mode2) {
    if (mode1 == modeRt || mode2 == modeRt)
        return modeRt;
    if (mode1 == modeUnspecified)
        return mode2;
    return mode1;
}

TypeRef Feather_adjustMode(TypeRef srcType, CompilationContext* context, Location loc) {
    ASSERT(srcType);
    ASSERT(context);
    EvalMode resMode = Feather_combineMode(srcType->mode, context->evalMode);
    return Feather_checkChangeTypeMode(srcType, resMode, loc);
}

void _printContextNodes(Node* node) {
    CompilationContext* ctx = node->context;
    while (ctx) {
        Node* n = ctx->currentSymTab->node;
        Node* expl = Nest_explanation(n);
        if (expl && (expl->nodeKind == Feather_getFirstFeatherNodeKind() + nkRelFeatherDeclClass ||
                            expl->nodeKind ==
                                    Feather_getFirstFeatherNodeKind() + nkRelFeatherDeclFunction))
            REP_INFO(expl->location, "In context: %1%") % NodeHandle(expl);

        ctx = ctx->parent;
    }
}

void Feather_checkEvalMode(Node* src) {
    ASSERT(src && src->type);
    EvalMode nodeEvalMode = src->type->mode;
    EvalMode contextEvalMode = src->context->evalMode;

    // Check if the context eval mode requirements are fulfilled
    if (contextEvalMode == modeCt && nodeEvalMode != modeCt) {
        _printContextNodes(src);
        REP_INTERNAL(src->location, "Cannot have non-CT nodes in a CT context (%1%)") %
                Nest_toStringEx(src);
    }

    // If we have a CT eval mode, then all the children must be CT
    if (nodeEvalMode == modeCt) {
        for (Node* child : src->children) {
            if (!child || !child->type)
                continue;

            // Ignore declarations
            if (_isDecl(child))
                continue;

            if (child->type->mode != modeCt)
                REP_INTERNAL(child->location,
                        "Children of a CT node must be CT; current mode: %1% (%2%)") %
                        child->type->mode % child;
        }
    }
}

void Feather_checkEvalModeWithExpected(Node* src, EvalMode referencedEvalMode) {
    Feather_checkEvalMode(src);

    ASSERT(src && src->type);
    EvalMode nodeEvalMode = src->type->mode;

    // Check if the referenced eval mode requirements are fulfilled
    if (referencedEvalMode == modeCt && nodeEvalMode != modeCt)
        REP_INTERNAL(src->location, "CT node required; found: %1% (%2%)") % nodeEvalMode %
                Nest_toStringEx(src);
}
