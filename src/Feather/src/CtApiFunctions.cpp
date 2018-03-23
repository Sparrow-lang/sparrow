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
    const auto* sourceCode = (const SourceCode*)thisArg->sourceCode;
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

template <typename T> void ctApiReg(Backend* backend, const char* name, T ftor) {
    backend->ctApiRegisterFun(
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
            backend, name, (void*)ftor);
}

} // namespace

void Feather_registerCtApiFunctions(Backend* backend) {
    ctApiReg(backend, "$meta.SourceCode.fromFilename", &ctApi_SourceCode_fromFilename);
    ctApiReg(backend, "$meta.SourceCode.filename", &ctApi_SourceCode_filename);

    ctApiReg(backend, "$meta.Location.getCorrespondingCode", &ctApi_Location_getCorrespondingCode);

    ctApiReg(backend, "$meta.report", &ctApi_report);
    ctApiReg(backend, "$meta.raise", &ctApi_raise);

    ctApiReg(backend, "$meta.CompilationContext.evalMode", &ctApi_CompilationContext_evalMode);
    ctApiReg(backend, "$meta.CompilationContext.sourceCode", &ctApi_CompilationContext_sourceCode);

    ctApiReg(backend, "$meta.AstType.typeKind", &ctApi_AstType_typeKind);
    ctApiReg(backend, "$meta.AstType.toString", &ctApi_AstType_toString);
    ctApiReg(backend, "$meta.AstType.hasStorage", &ctApi_AstType_hasStorage);
    ctApiReg(backend, "$meta.AstType.numReferences", &ctApi_AstType_numReferences);
    ctApiReg(backend, "$meta.AstType.mode", &ctApi_AstType_mode);
    ctApiReg(backend, "$meta.AstType.canBeUsedAtCt", &ctApi_AstType_canBeUsedAtCt);
    ctApiReg(backend, "$meta.AstType.canBeUsedAtRt", &ctApi_AstType_canBeUsedAtRt);

    ctApiReg(backend, "$meta.AstNode.clone", &ctApi_AstNode_clone);
    ctApiReg(backend, "$meta.AstNode.nodeKind", &ctApi_AstNode_nodeKind);
    ctApiReg(backend, "$meta.AstNode.nodeKindName", &ctApi_AstNode_nodeKindName);
    ctApiReg(backend, "$meta.AstNode.toString", &ctApi_AstNode_toString);
    ctApiReg(backend, "$meta.AstNode.toStringExt", &ctApi_AstNode_toStringExt);
    ctApiReg(backend, "$meta.AstNode.location", &ctApi_AstNode_location);
    ctApiReg(backend, "$meta.AstNode.children", &ctApi_AstNode_children);
    ctApiReg(backend, "$meta.AstNode.getChild", &ctApi_AstNode_getChild);
    ctApiReg(backend, "$meta.AstNode.referredNodes", &ctApi_AstNode_referredNodes);
    ctApiReg(backend, "$meta.AstNode.hasProperty", &ctApi_AstNode_hasProperty);
    ctApiReg(backend, "$meta.AstNode.getPropertyString", &ctApi_AstNode_getPropertyString);
    ctApiReg(backend, "$meta.AstNode.getPropertyInt", &ctApi_AstNode_getPropertyInt);
    ctApiReg(backend, "$meta.AstNode.getPropertyNode", &ctApi_AstNode_getPropertyNode);
    ctApiReg(backend, "$meta.AstNode.getPropertyType", &ctApi_AstNode_getPropertyType);
    ctApiReg(backend, "$meta.AstNode.setPropertyString", &ctApi_AstNode_setPropertyString);
    ctApiReg(backend, "$meta.AstNode.setPropertyInt", &ctApi_AstNode_setPropertyInt);
    ctApiReg(backend, "$meta.AstNode.setPropertyNode", &ctApi_AstNode_setPropertyNode);
    ctApiReg(backend, "$meta.AstNode.setPropertyType", &ctApi_AstNode_setPropertyType);
    ctApiReg(backend, "$meta.AstNode.setContext", &ctApi_AstNode_setContext);
    ctApiReg(backend, "$meta.AstNode.computeType", &ctApi_AstNode_computeType);
    ctApiReg(backend, "$meta.AstNode.semanticCheck", &ctApi_AstNode_semanticCheck);
    ctApiReg(backend, "$meta.AstNode.clearCompilationState", &ctApi_AstNode_clearCompilationState);
    ctApiReg(backend, "$meta.AstNode.context", &ctApi_AstNode_context);
    ctApiReg(backend, "$meta.AstNode.hasError", &ctApi_AstNode_hasError);
    ctApiReg(backend, "$meta.AstNode.isSemanticallyChecked", &ctApi_AstNode_isSemanticallyChecked);
    ctApiReg(backend, "$meta.AstNode.type", &ctApi_AstNode_type);
    ctApiReg(backend, "$meta.AstNode.isExplained", &ctApi_AstNode_isExplained);
    ctApiReg(backend, "$meta.AstNode.explanation", &ctApi_AstNode_explanation);
    ctApiReg(backend, "$meta.AstNode.curExplanation", &ctApi_AstNode_curExplanation);

    ctApiReg(backend, "$meta.Feather.mkNodeList", &ctApi_Feather_mkNodeList);
    ctApiReg(backend, "$meta.Feather.addToNodeList", &ctApi_Feather_addToNodeList);
    ctApiReg(backend, "$meta.Feather.appendNodeList", &ctApi_Feather_appendNodeList);
    ctApiReg(backend, "$meta.Feather.mkNop", &ctApi_Feather_mkNop);
    ctApiReg(backend, "$meta.Feather.mkTypeNode", &ctApi_Feather_mkTypeNode);
    ctApiReg(backend, "$meta.Feather.mkBackendCode", &ctApi_Feather_mkBackendCode);
    ctApiReg(backend, "$meta.Feather.mkLocalSpace", &ctApi_Feather_mkLocalSpace);
    ctApiReg(backend, "$meta.Feather.mkGlobalConstructAction",
            &ctApi_Feather_mkGlobalConstructAction);
    ctApiReg(
            backend, "$meta.Feather.mkGlobalDestructAction", &ctApi_Feather_mkGlobalDestructAction);
    ctApiReg(backend, "$meta.Feather.mkScopeDestructAction", &ctApi_Feather_mkScopeDestructAction);
    ctApiReg(backend, "$meta.Feather.mkTempDestructAction", &ctApi_Feather_mkTempDestructAction);
    ctApiReg(backend, "$meta.Feather.mkFunction", &ctApi_Feather_mkFunction);
    ctApiReg(backend, "$meta.Feather.mkClass", &ctApi_Feather_mkClass);
    ctApiReg(backend, "$meta.Feather.mkVar", &ctApi_Feather_mkVar);
    ctApiReg(backend, "$meta.Feather.mkCtValue", &ctApi_Feather_mkCtValue);
    ctApiReg(backend, "$meta.Feather.mkNull", &ctApi_Feather_mkNull);
    ctApiReg(backend, "$meta.Feather.mkVarRef", &ctApi_Feather_mkVarRef);
    ctApiReg(backend, "$meta.Feather.mkFieldRef", &ctApi_Feather_mkFieldRef);
    ctApiReg(backend, "$meta.Feather.mkFunRef", &ctApi_Feather_mkFunRef);
    ctApiReg(backend, "$meta.Feather.mkFunCall", &ctApi_Feather_mkFunCall);
    ctApiReg(backend, "$meta.Feather.mkMemLoad", &ctApi_Feather_mkMemLoad);
    ctApiReg(backend, "$meta.Feather.mkMemStore", &ctApi_Feather_mkMemStore);
    ctApiReg(backend, "$meta.Feather.mkBitcast", &ctApi_Feather_mkBitcast);
    ctApiReg(backend, "$meta.Feather.mkConditional", &ctApi_Feather_mkConditional);
    ctApiReg(backend, "$meta.Feather.mkIf", &ctApi_Feather_mkIf);
    ctApiReg(backend, "$meta.Feather.mkWhile", &ctApi_Feather_mkWhile);
    ctApiReg(backend, "$meta.Feather.mkBreak", &ctApi_Feather_mkBreak);
    ctApiReg(backend, "$meta.Feather.mkContinue", &ctApi_Feather_mkContinue);
    ctApiReg(backend, "$meta.Feather.mkReturn", &ctApi_Feather_mkReturn);
}
