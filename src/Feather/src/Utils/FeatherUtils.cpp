#include "Feather/src/StdInc.h"
#include "Feather/Utils/FeatherUtils.hpp"
#include "Feather/Utils/cppif/FeatherTypes.hpp"

#include "Feather/Api/Feather.h"
#include "Nest/Api/Node.h"
#include "Nest/Api/CompilationContext.h"
#include "Nest/Api/SymTab.h"
#include "Nest/Utils/cppif/NodeHandle.hpp"
#include "Nest/Utils/cppif/NodeUtils.hpp"
#include "Nest/Utils/cppif/StringRef.hpp"
#include "Nest/Utils/Diagnostic.hpp"

using Nest::NodeHandle;
using namespace Feather;

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

int _isTestable(Type type) {
    // If not Testable, check at least that is some kind of boolean
    if (!type || !type.hasStorage())
        return false;
    StringRef nativeName = Feather_nativeName(type);
    return nativeName && (nativeName == "i1" || nativeName == "u1");
}

int Feather_isTestable(Node* node) { return _isTestable(node->type); }

int Feather_isBasicNumericType(Nest::TypeRef type) {
    if (!type || !type->hasStorage)
        return false;
    StringRef nativeName = Feather_nativeName(type);
    return !nativeName.empty() &&
           (nativeName == "i1" || nativeName == "u1" || nativeName == "i8" || nativeName == "u8" ||
                   nativeName == "i16" || nativeName == "u16" || nativeName == "i32" ||
                   nativeName == "u32" || nativeName == "i64" || nativeName == "u64" ||
                   nativeName == "float" || nativeName == "double");
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

Nest::TypeRef Feather_adjustMode(Nest::TypeRef srcType, CompilationContext* context, Location loc) {
    ASSERT(srcType);
    ASSERT(context);
    EvalMode resMode = Feather_combineMode(srcType->mode, context->evalMode);
    return Type(srcType).changeMode(resMode, loc);
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
                REP_INTERNAL(child->location, "Children of a CT node must be CT; current mode: %1% "
                                              "(child: %2%: %3%, parent: %4%: %5%)") %
                        child->type->mode % child % child->type % src % src->type;
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
