#include "Feather/Utils/FeatherUtils.h"

#include "Nest/Api/CompilationContext.h"
#include "Nest/Api/SymTab.h"
#include "Nest/Api/Type.h"
#include "Nest/Utils/NodeUtils.h"
#include "Nest/Utils/Diagnostic.h"

const char* propNativeName = "nativeName";
const char* propDescription = "description";
const char* propResultParam = "resultParam";
const char* propAutoCt = "autoCt";
const char* propCode = "code";
const char* propEvalMode = "evalMode";
const char* propResultingDecl = "resultingDecl";
const char* propNoInline = "noInline";
const char* propEmptyBody = "emptyBody";
const char* propTypeCat = "typeCat";

/// Tests if the given node is a declaration (a node that will expand to a Feather declaration)
int _isDecl(Nest_Node* node) {
    switch (Nest_explanation(node)->nodeKind - Feather_getFirstFeatherNodeKind()) {
    case nkRelFeatherDeclFunction:
    case nkRelFeatherDeclClass:
    case nkRelFeatherDeclVar:
        return 1;
    default:
        return 0;
    }
}

Nest_Node* _getParentDecl(Nest_CompilationContext* context) {
    Nest_SymTab* parentSymTab = context->currentSymTab;
    for (; parentSymTab; parentSymTab = parentSymTab->parent) {
        Nest_Node* n = parentSymTab->node;
        if (n && _isDecl(n))
            return n;
    }
    return 0;
}

Nest_Node* Feather_classDecl(Nest_TypeRef type) {
    ASSERT(type && type->hasStorage);
    return type->referredNode;
}

Nest_StringRef Feather_nativeName(Nest_TypeRef type) {
    Nest_StringRef res = {0, 0};
    if (type->referredNode && type->referredNode->nodeKind == nkFeatherDeclClass)
        res = Nest_getPropertyStringDeref(type->referredNode, propNativeName);
    return res;
}

Nest_TypeRef Feather_baseType(Nest_TypeRef type) {
    ASSERT(type);
    ASSERT(type->typeKind == typeKindConst || type->typeKind == typeKindMutable ||
            type->typeKind == typeKindTemp || type->typeKind == typeKindArray);
    ASSERT(type->numSubtypes == 1);
    return type->subTypes[0];
}

int Feather_getArraySize(Nest_TypeRef type) {
    ASSERT(type && type->typeKind == typeKindArray);
    return type->flags;
}

unsigned Feather_numFunParameters(Nest_TypeRef type) {
    ASSERT(type && type->typeKind == typeKindFunction);
    return type->numSubtypes - 1;
}

Nest_TypeRef Feather_getFunParameter(Nest_TypeRef type, unsigned idx) {
    ASSERT(type && type->typeKind == typeKindFunction);
    return type->subTypes[idx + 1];
}

Nest_TypeRef Feather_getFunResultType(Nest_TypeRef type) {
    ASSERT(type && type->typeKind == typeKindFunction);
    return type->subTypes[0];
}

Nest_Node* Feather_getParentFun(Nest_CompilationContext* context) {
    return Nest_ofKind(Nest_explanation(_getParentDecl(context)), nkFeatherDeclFunction);
}

Nest_Node* Feather_getParentClass(Nest_CompilationContext* context) {
    return Nest_ofKind(Nest_explanation(_getParentDecl(context)), nkFeatherDeclClass);
}

Nest_Node* Feather_getParentLoop(Nest_CompilationContext* context) {
    Nest_SymTab* parentSymTab = context->currentSymTab;
    for (; parentSymTab; parentSymTab = parentSymTab->parent) {
        Nest_Node* n = parentSymTab->node;

        // Do we have a while node?
        Nest_Node* expl = Nest_explanation(n);
        if (expl->nodeKind == nkFeatherStmtWhile)
            return expl;

        // Stop if we encounter a declaration
        if (_isDecl(n))
            return 0;
    }
    return 0;
}

Nest_StringRef Feather_getName(Nest_Node* decl) {
    return Nest_getCheckPropertyString(decl, "name");
}

int Feather_hasName(Nest_Node* decl) { return Nest_hasProperty(decl, "name"); }

void Feather_setName(Nest_Node* decl, Nest_StringRef name) {
    Nest_setPropertyString(decl, "name", name);
}

EvalMode Feather_nodeEvalMode(Nest_Node* decl) {
    const int* val = Nest_getPropertyInt(decl, "evalMode");
    return val ? (EvalMode)*val : modeUnspecified;
}
EvalMode Feather_effectiveEvalMode(Nest_Node* decl) {
    EvalMode nodeMode = Feather_nodeEvalMode(decl);
    return nodeMode != modeUnspecified ? nodeMode : decl->context->evalMode;
}
void Feather_setEvalMode(Nest_Node* decl, EvalMode val) {
    Nest_setPropertyInt(decl, "evalMode", val);

    // Sanity check
    //    EvalMode curMode = declEvalMode(this);
    //    if ( data_.childrenContext && curMode != modeUnspecified &&
    //    data_.childrenContext->evalMode() != curMode )
    //        REP_INTERNAL(data_.location, "Invalid mode set for node; node has %1%, in context
    //        %2%") % curMode % data_.childrenContext->evalMode();
}

void Feather_addToSymTab(Nest_Node* decl) {
    const int* dontAddToSymTab = Nest_getPropertyInt(decl, "dontAddToSymTab");
    if (dontAddToSymTab && *dontAddToSymTab)
        return;

    if (!decl->context)
        Nest_reportFmt(decl->location, diagInternalError,
                "Cannot add node %s to sym-tab: context is not set", Nest_nodeKindName(decl));
    ASSERT(decl->context);
    Nest_StringRef declName = Feather_getName(decl);
    if (declName.begin == declName.end)
        Nest_reportFmt(decl->location, diagInternalError,
                "Cannot add node %s to sym-tab: no name set", Nest_nodeKindName(decl));
    Nest_symTabEnter(decl->context->currentSymTab, declName.begin, decl);
}

void Feather_setShouldAddToSymTab(Nest_Node* decl, int shouldAdd) {
    Nest_setPropertyInt(decl, "dontAddToSymTab", shouldAdd ? 0 : 1);
}
