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

/// Tests if the given node is a declaration (a node that will expand to a Feather declaration)
int _isDecl(Node* node) {
    switch (Nest_explanation(node)->nodeKind - Feather_getFirstFeatherNodeKind()) {
    case nkRelFeatherDeclFunction:
    case nkRelFeatherDeclClass:
    case nkRelFeatherDeclVar:
        return 1;
    default:
        return 0;
    }
}

Node* _getParentDecl(CompilationContext* context) {
    SymTab* parentSymTab = context->currentSymTab;
    for (; parentSymTab; parentSymTab = parentSymTab->parent) {
        Node* n = parentSymTab->node;
        if (n && _isDecl(n))
            return n;
    }
    return 0;
}

Node* Feather_classDecl(TypeRef type) {
    ASSERT(type && type->hasStorage);
    return type->referredNode;
}

StringRef Feather_nativeName(TypeRef type) {
    StringRef res = {0, 0};
    if (type->referredNode && type->referredNode->nodeKind == nkFeatherDeclClass)
        res = Nest_getPropertyStringDeref(type->referredNode, propNativeName);
    return res;
}

int Feather_numReferences(TypeRef type) {
    ASSERT(type && type->hasStorage);
    return type->numReferences;
}

TypeRef Feather_baseType(TypeRef type) {
    ASSERT(type && (type->typeKind == typeKindLValue || type->typeKind == typeKindArray) &&
            type->numSubtypes == 1);
    return type->subTypes[0];
}

int Feather_getArraySize(TypeRef type) {
    ASSERT(type && type->typeKind == typeKindArray);
    return type->flags;
}

unsigned Feather_numFunParameters(TypeRef type) {
    ASSERT(type && type->typeKind == typeKindFunction);
    return type->numSubtypes - 1;
}

TypeRef Feather_getFunParameter(TypeRef type, unsigned idx) {
    ASSERT(type && type->typeKind == typeKindFunction);
    return type->subTypes[idx + 1];
}

TypeRef Feather_getFunResultType(TypeRef type) {
    ASSERT(type && type->typeKind == typeKindFunction);
    return type->subTypes[0];
}

Node* Feather_getParentFun(CompilationContext* context) {
    return Nest_ofKind(Nest_explanation(_getParentDecl(context)), nkFeatherDeclFunction);
}

Node* Feather_getParentClass(CompilationContext* context) {
    return Nest_ofKind(Nest_explanation(_getParentDecl(context)), nkFeatherDeclClass);
}

Node* Feather_getParentLoop(CompilationContext* context) {
    SymTab* parentSymTab = context->currentSymTab;
    for (; parentSymTab; parentSymTab = parentSymTab->parent) {
        Node* n = parentSymTab->node;

        // Do we have a while node?
        Node* expl = Nest_explanation(n);
        if (expl->nodeKind == nkFeatherStmtWhile)
            return expl;

        // Stop if we encounter a declaration
        if (_isDecl(n))
            return 0;
    }
    return 0;
}

StringRef Feather_getName(const Node* decl) { return Nest_getCheckPropertyString(decl, "name"); }

int Feather_hasName(const Node* decl) { return Nest_hasProperty(decl, "name"); }

void Feather_setName(Node* decl, StringRef name) { Nest_setPropertyString(decl, "name", name); }

EvalMode Feather_nodeEvalMode(const Node* decl) {
    const int* val = Nest_getPropertyInt(decl, "evalMode");
    return val ? (EvalMode)*val : modeUnspecified;
}
EvalMode Feather_effectiveEvalMode(const Node* decl) {
    EvalMode nodeMode = Feather_nodeEvalMode(decl);
    return nodeMode != modeUnspecified ? nodeMode : decl->context->evalMode;
}
void Feather_setEvalMode(Node* decl, EvalMode val) {
    Nest_setPropertyInt(decl, "evalMode", val);

    // Sanity check
    //    EvalMode curMode = declEvalMode(this);
    //    if ( data_.childrenContext && curMode != modeUnspecified &&
    //    data_.childrenContext->evalMode() != curMode )
    //        REP_INTERNAL(data_.location, "Invalid mode set for node; node has %1%, in context
    //        %2%") % curMode % data_.childrenContext->evalMode();
}

void Feather_addToSymTab(Node* decl) {
    const int* dontAddToSymTab = Nest_getPropertyInt(decl, "dontAddToSymTab");
    if (dontAddToSymTab && *dontAddToSymTab)
        return;

    if (!decl->context)
        Nest_reportFmt(decl->location, diagInternalError,
                "Cannot add node %s to sym-tab: context is not set", Nest_nodeKindName(decl));
    ASSERT(decl->context);
    StringRef declName = Feather_getName(decl);
    if (declName.begin == declName.end)
        Nest_reportFmt(decl->location, diagInternalError,
                "Cannot add node %s to sym-tab: no name set", Nest_nodeKindName(decl));
    Nest_symTabEnter(decl->context->currentSymTab, declName.begin, decl);
}

void Feather_setShouldAddToSymTab(Node* decl, int shouldAdd) {
    Nest_setPropertyInt(decl, "dontAddToSymTab", shouldAdd ? 0 : 1);
}
