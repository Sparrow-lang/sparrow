#include "Feather/src/StdInc.h"
#include "CtApiFunctions.h"

#include "Feather/Api/Feather.h"
#include "Feather/Utils/FeatherUtils.hpp"

#include "Nest/Api/Node.h"
#include "Nest/Api/Backend.h"
#include "Nest/Api/SourceCode.h"
#include "Nest/Utils/Diagnostic.hpp"
#include "Nest/Utils/NodeUtils.h"
#include "Nest/Utils/StringRef.hpp"

using namespace Nest;

namespace {
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Location, SourceCode, diagnostic
//
const SourceCode* ctApi_SourceCode_fromFilename(StringRef filename) {
    return Nest_getSourceCodeForFilename(filename);
}
StringRef ctApi_SourceCode_filename(SourceCode* thisArg) { return fromCStr(thisArg->url); }
StringRef ctApi_Location_getCorrespondingCode(Location* thisArg) {
    const SourceCode* sourceCode = (const SourceCode*)thisArg->sourceCode;
    string code;
    StringRef lineStr = {nullptr, nullptr};
    if (sourceCode &&
            !(thisArg->start.line == thisArg->end.line && thisArg->start.col > thisArg->end.col)) {
        lineStr = Nest_getSourceCodeLine(sourceCode, thisArg->start.line);
        size_t len = lineStr.end - lineStr.begin;
        size_t count = thisArg->end.line == thisArg->start.line
                               ? thisArg->end.col - thisArg->start.col
                               : len - thisArg->start.col; // first line until the end
        lineStr.begin += thisArg->start.col - 1;
        lineStr.end = lineStr.begin + count;
    }
    return lineStr;
}

void ctApi_report(int type, Location* location, StringRef message) {
    Location loc = location ? *location : Location();
    Nest_reportDiagnostic(loc, (DiagnosticSeverity)type, toString(message).c_str());
}
void ctApi_raise() {
    REP_INFO(NOLOC, "CT API invoked raise. Exiting...");
    exit(0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CompilationContext
//
int ctApi_CompilationContext_evalMode(CompilationContext* thisArg) { return thisArg->evalMode; }
SourceCode* ctApi_CompilationContext_sourceCode(CompilationContext* thisArg) {
    return thisArg->sourceCode;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// AstType
//
int ctApi_AstType_typeKind(TypeRef thisArg) { return thisArg->typeKind; }
StringRef ctApi_AstType_toString(TypeRef thisArg) { return dupCStr(thisArg->description); }
bool ctApi_AstType_hasStorage(TypeRef thisArg) { return thisArg->hasStorage; }
int ctApi_AstType_numReferences(TypeRef thisArg) { return thisArg->numReferences; }
int ctApi_AstType_mode(TypeRef thisArg) { return thisArg->mode; }
bool ctApi_AstType_canBeUsedAtCt(TypeRef thisArg) { return thisArg->canBeUsedAtCt; }
bool ctApi_AstType_canBeUsedAtRt(TypeRef thisArg) { return thisArg->canBeUsedAtRt; }

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// AstNode
//
Node* ctApi_AstNode_clone(Node* thisArg) { return Nest_cloneNode(thisArg); }
int ctApi_AstNode_nodeKind(Node* thisArg) { return thisArg->nodeKind; }
StringRef ctApi_AstNode_nodeKindName(Node* thisArg) { return fromCStr(Nest_nodeKindName(thisArg)); }
StringRef ctApi_AstNode_toString(Node* thisArg) { return fromCStr(Nest_toString(thisArg)); }
StringRef ctApi_AstNode_toStringExt(Node* thisArg) { return fromCStr(Nest_toString(thisArg)); }
Location ctApi_AstNode_location(Node* thisArg) { return thisArg->location; }
NodeRange ctApi_AstNode_children(Node* thisArg) { return Nest_nodeChildren(thisArg); }
Node* ctApi_AstNode_getChild(Node* thisArg, int n) {
    NodeRange r = Nest_nodeChildren(thisArg);
    return r.beginPtr[n];
}
NodeRange ctApi_AstNode_referredNodes(Node* thisArg, Node*** retBegin, Node*** retEnd) {
    return Nest_nodeReferredNodes(thisArg);
}
bool ctApi_AstNode_hasProperty(Node* thisArg, StringRef name) {
    return Nest_hasProperty(thisArg, name.begin);
}
bool ctApi_AstNode_getPropertyString(Node* thisArg, StringRef name, StringRef* value) {
    const StringRef* res = Nest_getPropertyString(thisArg, name.begin);
    if (res) {
        *value = *res;
    }
    return res != nullptr;
}
bool ctApi_AstNode_getPropertyInt(Node* thisArg, StringRef name, int* value) {
    const int* res = Nest_getPropertyInt(thisArg, name.begin);
    if (res)
        *value = *res;
    return res != nullptr;
}
bool ctApi_AstNode_getPropertyNode(Node* thisArg, StringRef name, Node** value) {
    Node* const* res = Nest_getPropertyNode(thisArg, name.begin);
    if (res)
        *value = *res;
    return res != nullptr;
}
bool ctApi_AstNode_getPropertyType(Node* thisArg, StringRef name, TypeRef* value) {
    const TypeRef* res = Nest_getPropertyType(thisArg, name.begin);
    if (res)
        *value = *res;
    return res != nullptr;
}
void ctApi_AstNode_setPropertyString(Node* thisArg, StringRef name, StringRef value) {
    Nest_setPropertyString(thisArg, name.begin, value);
}
void ctApi_AstNode_setPropertyInt(Node* thisArg, StringRef name, int value) {
    Nest_setPropertyInt(thisArg, name.begin, value);
}
void ctApi_AstNode_setPropertyNode(Node* thisArg, StringRef name, Node* value) {
    Nest_setPropertyNode(thisArg, name.begin, value);
}
void ctApi_AstNode_setPropertyType(Node* thisArg, StringRef name, TypeRef value) {
    Nest_setPropertyType(thisArg, name.begin, value);
}
void ctApi_AstNode_setContext(Node* thisArg, CompilationContext* context) {
    Nest_setContext(thisArg, context);
}
void ctApi_AstNode_computeType(Node* thisArg) { Nest_computeType(thisArg); }
void ctApi_AstNode_semanticCheck(Node* thisArg) { Nest_semanticCheck(thisArg); }
void ctApi_AstNode_clearCompilationState(Node* thisArg) { Nest_clearCompilationState(thisArg); }
CompilationContext* ctApi_AstNode_context(Node* thisArg) { return thisArg->context; }
bool ctApi_AstNode_hasError(Node* thisArg) { return thisArg->nodeError != 0; }
bool ctApi_AstNode_isSemanticallyChecked(Node* thisArg) {
    return thisArg->nodeSemanticallyChecked != 0;
}
TypeRef ctApi_AstNode_type(Node* thisArg) { return thisArg->type; }
bool ctApi_AstNode_isExplained(Node* thisArg) { return thisArg->explanation != nullptr; }
Node* ctApi_AstNode_explanation(Node* thisArg) { return Nest_explanation(thisArg); }
Node* ctApi_AstNode_curExplanation(Node* thisArg) { return thisArg->explanation; }

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Feather nodes creation functions
//
Node* ctApi_Feather_mkNodeList(Location* loc, NodeRange children, bool voidResult) {
    if (voidResult)
        return Feather_mkNodeListVoid(*loc, children);
    else
        return Feather_mkNodeList(*loc, children);
}
Node* ctApi_Feather_addToNodeList(Node* prevList, Node* element) {
    return Feather_addToNodeList(prevList, element);
}
Node* ctApi_Feather_appendNodeList(Node* list, Node* newNodes) {
    return Feather_appendNodeList(list, newNodes);
}

Node* ctApi_Feather_mkNop(Location* loc) { return Feather_mkNop(*loc); }
Node* ctApi_Feather_mkTypeNode(Location* loc, TypeRef type) {
    return Feather_mkTypeNode(*loc, type);
}
Node* ctApi_Feather_mkBackendCode(Location* loc, StringRef code, int evalMode) {
    return Feather_mkBackendCode(*loc, code, (EvalMode)evalMode);
}
Node* ctApi_Feather_mkLocalSpace(Location* loc, NodeRange children) {
    return Feather_mkLocalSpace(*loc, children);
}
Node* ctApi_Feather_mkGlobalConstructAction(Location* loc, Node* action) {
    return Feather_mkGlobalConstructAction(*loc, action);
}
Node* ctApi_Feather_mkGlobalDestructAction(Location* loc, Node* action) {
    return Feather_mkGlobalDestructAction(*loc, action);
}
Node* ctApi_Feather_mkScopeDestructAction(Location* loc, Node* action) {
    return Feather_mkScopeDestructAction(*loc, action);
}
Node* ctApi_Feather_mkTempDestructAction(Location* loc, Node* action) {
    return Feather_mkTempDestructAction(*loc, action);
}

Node* ctApi_Feather_mkFunction(
        Location* loc, StringRef name, Node* resType, NodeRange params, Node* body, int evalMode) {
    return Feather_mkFunction(*loc, name, resType, params, body);
}
Node* ctApi_Feather_mkClass(Location* loc, StringRef name, NodeRange fields, int evalMode) {
    return Feather_mkClass(*loc, name, fields);
}
Node* ctApi_Feather_mkVar(Location* loc, StringRef name, Node* type, int evalMode) {
    Node* res = Feather_mkVar(*loc, name, type);
    Feather_setEvalMode(res, (EvalMode)evalMode);
    return res;
}

Node* ctApi_Feather_mkCtValue(Location* loc, TypeRef type, StringRef data) {
    return Feather_mkCtValue(*loc, type, data);
}
Node* ctApi_Feather_mkNull(Location* loc, Node* typeNode) { return Feather_mkNull(*loc, typeNode); }
Node* ctApi_Feather_mkVarRef(Location* loc, Node* varDecl) {
    return Feather_mkVarRef(*loc, varDecl);
}
Node* ctApi_Feather_mkFieldRef(Location* loc, Node* obj, Node* fieldDecl) {
    return Feather_mkFieldRef(*loc, obj, fieldDecl);
}
Node* ctApi_Feather_mkFunRef(Location* loc, Node* funDecl, Node* resType) {
    return Feather_mkFunRef(*loc, funDecl, resType);
}
Node* ctApi_Feather_mkFunCall(Location* loc, Node* funDecl, NodeRange args) {
    return Feather_mkFunCall(*loc, funDecl, args);
}
Node* ctApi_Feather_mkMemLoad(Location* loc, Node* exp) { return Feather_mkMemLoad(*loc, exp); }
Node* ctApi_Feather_mkMemStore(Location* loc, Node* value, Node* address) {
    return Feather_mkMemStore(*loc, value, address);
}
Node* ctApi_Feather_mkBitcast(Location* loc, Node* destType, Node* exp) {
    return Feather_mkBitcast(*loc, destType, exp);
}
Node* ctApi_Feather_mkConditional(Location* loc, Node* condition, Node* alt1, Node* alt2) {
    return Feather_mkConditional(*loc, condition, alt1, alt2);
}

Node* ctApi_Feather_mkIf(
        Location* loc, Node* condition, Node* thenClause, Node* elseClause, bool isCt) {
    Node* res = Feather_mkIf(*loc, condition, thenClause, elseClause);
    if (isCt)
        Feather_setEvalMode(res, modeCt);
    return res;
}
Node* ctApi_Feather_mkWhile(Location* loc, Node* condition, Node* body, Node* step, bool isCt) {
    Node* res = Feather_mkWhile(*loc, condition, body, step);
    if (isCt)
        Feather_setEvalMode(res, modeCt);
    return res;
}
Node* ctApi_Feather_mkBreak(Location* loc) { return Feather_mkBreak(*loc); }
Node* ctApi_Feather_mkContinue(Location* loc) { return Feather_mkContinue(*loc); }
Node* ctApi_Feather_mkReturn(Location* loc, Node* exp) { return Feather_mkReturn(*loc, exp); }
} // namespace

void Feather_registerCtApiFunctions(Backend* backend) {
    backend->ctApiRegisterFun(
            backend, "$meta.SourceCode.fromFilename", (void*)&ctApi_SourceCode_fromFilename);
    backend->ctApiRegisterFun(
            backend, "$meta.SourceCode.filename", (void*)&ctApi_SourceCode_filename);

    backend->ctApiRegisterFun(backend, "$meta.Location.getCorrespondingCode",
            (void*)&ctApi_Location_getCorrespondingCode);

    backend->ctApiRegisterFun(backend, "$meta.report", (void*)&ctApi_report);
    backend->ctApiRegisterFun(backend, "$meta.raise", (void*)&ctApi_raise);

    backend->ctApiRegisterFun(backend, "$meta.CompilationContext.evalMode",
            (void*)&ctApi_CompilationContext_evalMode);
    backend->ctApiRegisterFun(backend, "$meta.CompilationContext.sourceCode",
            (void*)&ctApi_CompilationContext_sourceCode);

    backend->ctApiRegisterFun(backend, "$meta.AstType.typeKind", (void*)&ctApi_AstType_typeKind);
    backend->ctApiRegisterFun(backend, "$meta.AstType.toString", (void*)&ctApi_AstType_toString);
    backend->ctApiRegisterFun(
            backend, "$meta.AstType.hasStorage", (void*)&ctApi_AstType_hasStorage);
    backend->ctApiRegisterFun(
            backend, "$meta.AstType.numReferences", (void*)&ctApi_AstType_numReferences);
    backend->ctApiRegisterFun(backend, "$meta.AstType.mode", (void*)&ctApi_AstType_mode);
    backend->ctApiRegisterFun(
            backend, "$meta.AstType.canBeUsedAtCt", (void*)&ctApi_AstType_canBeUsedAtCt);
    backend->ctApiRegisterFun(
            backend, "$meta.AstType.canBeUsedAtRt", (void*)&ctApi_AstType_canBeUsedAtRt);

    backend->ctApiRegisterFun(backend, "$meta.AstNode.clone", (void*)&ctApi_AstNode_clone);
    backend->ctApiRegisterFun(backend, "$meta.AstNode.nodeKind", (void*)&ctApi_AstNode_nodeKind);
    backend->ctApiRegisterFun(
            backend, "$meta.AstNode.nodeKindName", (void*)&ctApi_AstNode_nodeKindName);
    backend->ctApiRegisterFun(backend, "$meta.AstNode.toString", (void*)&ctApi_AstNode_toString);
    backend->ctApiRegisterFun(
            backend, "$meta.AstNode.toStringExt", (void*)&ctApi_AstNode_toStringExt);
    backend->ctApiRegisterFun(backend, "$meta.AstNode.location", (void*)&ctApi_AstNode_location);
    backend->ctApiRegisterFun(backend, "$meta.AstNode.children", (void*)&ctApi_AstNode_children);
    backend->ctApiRegisterFun(backend, "$meta.AstNode.getChild", (void*)&ctApi_AstNode_getChild);
    backend->ctApiRegisterFun(
            backend, "$meta.AstNode.referredNodes", (void*)&ctApi_AstNode_referredNodes);
    backend->ctApiRegisterFun(
            backend, "$meta.AstNode.hasProperty", (void*)&ctApi_AstNode_hasProperty);
    backend->ctApiRegisterFun(
            backend, "$meta.AstNode.getPropertyString", (void*)&ctApi_AstNode_getPropertyString);
    backend->ctApiRegisterFun(
            backend, "$meta.AstNode.getPropertyInt", (void*)&ctApi_AstNode_getPropertyInt);
    backend->ctApiRegisterFun(
            backend, "$meta.AstNode.getPropertyNode", (void*)&ctApi_AstNode_getPropertyNode);
    backend->ctApiRegisterFun(
            backend, "$meta.AstNode.getPropertyType", (void*)&ctApi_AstNode_getPropertyType);
    backend->ctApiRegisterFun(
            backend, "$meta.AstNode.setPropertyString", (void*)&ctApi_AstNode_setPropertyString);
    backend->ctApiRegisterFun(
            backend, "$meta.AstNode.setPropertyInt", (void*)&ctApi_AstNode_setPropertyInt);
    backend->ctApiRegisterFun(
            backend, "$meta.AstNode.setPropertyNode", (void*)&ctApi_AstNode_setPropertyNode);
    backend->ctApiRegisterFun(
            backend, "$meta.AstNode.setPropertyType", (void*)&ctApi_AstNode_setPropertyType);
    backend->ctApiRegisterFun(
            backend, "$meta.AstNode.setContext", (void*)&ctApi_AstNode_setContext);
    backend->ctApiRegisterFun(
            backend, "$meta.AstNode.computeType", (void*)&ctApi_AstNode_computeType);
    backend->ctApiRegisterFun(
            backend, "$meta.AstNode.semanticCheck", (void*)&ctApi_AstNode_semanticCheck);
    backend->ctApiRegisterFun(backend, "$meta.AstNode.clearCompilationState",
            (void*)&ctApi_AstNode_clearCompilationState);
    backend->ctApiRegisterFun(backend, "$meta.AstNode.context", (void*)&ctApi_AstNode_context);
    backend->ctApiRegisterFun(backend, "$meta.AstNode.hasError", (void*)&ctApi_AstNode_hasError);
    backend->ctApiRegisterFun(backend, "$meta.AstNode.isSemanticallyChecked",
            (void*)&ctApi_AstNode_isSemanticallyChecked);
    backend->ctApiRegisterFun(backend, "$meta.AstNode.type", (void*)&ctApi_AstNode_type);
    backend->ctApiRegisterFun(
            backend, "$meta.AstNode.isExplained", (void*)&ctApi_AstNode_isExplained);
    backend->ctApiRegisterFun(
            backend, "$meta.AstNode.explanation", (void*)&ctApi_AstNode_explanation);
    backend->ctApiRegisterFun(
            backend, "$meta.AstNode.curExplanation", (void*)&ctApi_AstNode_curExplanation);

    backend->ctApiRegisterFun(
            backend, "$meta.Feather.mkNodeList", (void*)&ctApi_Feather_mkNodeList);
    backend->ctApiRegisterFun(
            backend, "$meta.Feather.addToNodeList", (void*)&ctApi_Feather_addToNodeList);
    backend->ctApiRegisterFun(
            backend, "$meta.Feather.appendNodeList", (void*)&ctApi_Feather_appendNodeList);
    backend->ctApiRegisterFun(backend, "$meta.Feather.mkNop", (void*)&ctApi_Feather_mkNop);
    backend->ctApiRegisterFun(
            backend, "$meta.Feather.mkTypeNode", (void*)&ctApi_Feather_mkTypeNode);
    backend->ctApiRegisterFun(
            backend, "$meta.Feather.mkBackendCode", (void*)&ctApi_Feather_mkBackendCode);
    backend->ctApiRegisterFun(
            backend, "$meta.Feather.mkLocalSpace", (void*)&ctApi_Feather_mkLocalSpace);
    backend->ctApiRegisterFun(backend, "$meta.Feather.mkGlobalConstructAction",
            (void*)&ctApi_Feather_mkGlobalConstructAction);
    backend->ctApiRegisterFun(backend, "$meta.Feather.mkGlobalDestructAction",
            (void*)&ctApi_Feather_mkGlobalDestructAction);
    backend->ctApiRegisterFun(backend, "$meta.Feather.mkScopeDestructAction",
            (void*)&ctApi_Feather_mkScopeDestructAction);
    backend->ctApiRegisterFun(backend, "$meta.Feather.mkTempDestructAction",
            (void*)&ctApi_Feather_mkTempDestructAction);
    backend->ctApiRegisterFun(
            backend, "$meta.Feather.mkFunction", (void*)&ctApi_Feather_mkFunction);
    backend->ctApiRegisterFun(backend, "$meta.Feather.mkClass", (void*)&ctApi_Feather_mkClass);
    backend->ctApiRegisterFun(backend, "$meta.Feather.mkVar", (void*)&ctApi_Feather_mkVar);
    backend->ctApiRegisterFun(backend, "$meta.Feather.mkCtValue", (void*)&ctApi_Feather_mkCtValue);
    backend->ctApiRegisterFun(backend, "$meta.Feather.mkNull", (void*)&ctApi_Feather_mkNull);
    backend->ctApiRegisterFun(backend, "$meta.Feather.mkVarRef", (void*)&ctApi_Feather_mkVarRef);
    backend->ctApiRegisterFun(
            backend, "$meta.Feather.mkFieldRef", (void*)&ctApi_Feather_mkFieldRef);
    backend->ctApiRegisterFun(backend, "$meta.Feather.mkFunRef", (void*)&ctApi_Feather_mkFunRef);
    backend->ctApiRegisterFun(backend, "$meta.Feather.mkFunCall", (void*)&ctApi_Feather_mkFunCall);
    backend->ctApiRegisterFun(backend, "$meta.Feather.mkMemLoad", (void*)&ctApi_Feather_mkMemLoad);
    backend->ctApiRegisterFun(
            backend, "$meta.Feather.mkMemStore", (void*)&ctApi_Feather_mkMemStore);
    backend->ctApiRegisterFun(backend, "$meta.Feather.mkBitcast", (void*)&ctApi_Feather_mkBitcast);
    backend->ctApiRegisterFun(
            backend, "$meta.Feather.mkConditional", (void*)&ctApi_Feather_mkConditional);
    backend->ctApiRegisterFun(backend, "$meta.Feather.mkIf", (void*)&ctApi_Feather_mkIf);
    backend->ctApiRegisterFun(backend, "$meta.Feather.mkWhile", (void*)&ctApi_Feather_mkWhile);
    backend->ctApiRegisterFun(backend, "$meta.Feather.mkBreak", (void*)&ctApi_Feather_mkBreak);
    backend->ctApiRegisterFun(
            backend, "$meta.Feather.mkContinue", (void*)&ctApi_Feather_mkContinue);
    backend->ctApiRegisterFun(backend, "$meta.Feather.mkReturn", (void*)&ctApi_Feather_mkReturn);
}
