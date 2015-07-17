#pragma once

#include <Nest/Intermediate/EvalMode.h>
#include <Feather/Nodes/DynNode.h>
#include <Feather/Nodes/Exp/AtomicOrdering.h>
#include <Feather/Nodes/Decls/CallConvention.h>

FWD_CLASS1(Feather, NodeList);

namespace Feather
{
    using namespace Nest;

    /// Called to initialize the Feather node kinds
    void initFeatherNodeKinds();

    NodeList* mkNodeList(const Location& loc, DynNodeVector children, bool voidResult = false);
    NodeList* addToNodeList(NodeList* prevList, DynNode* element);
    NodeList* appendNodeList(NodeList* list, NodeList* newNodes);

    DynNode* mkNop(const Location& loc);
    DynNode* mkTypeNode(const Location& loc, TypeRef type);
    DynNode* mkBackendCode(const Location& loc, string code, EvalMode evalMode = modeRt);
    DynNode* mkLocalSpace(const Location& loc, DynNodeVector children);
    DynNode* mkGlobalConstructAction(const Location& loc, DynNode* action);
    DynNode* mkGlobalDestructAction(const Location& loc, DynNode* action);
    DynNode* mkScopeDestructAction(const Location& loc, DynNode* action);
    DynNode* mkTempDestructAction(const Location& loc, DynNode* action);
    
    DynNode* mkFunction(const Location& loc, string name, DynNode* resType, DynNodeVector params, DynNode* body, CallConvention callConv = ccC, EvalMode evalMode = modeUnspecified);
    DynNode* mkClass(const Location& loc, string name, DynNodeVector fields, EvalMode evalMode = modeUnspecified);
    DynNode* mkVar(const Location& loc, string name, DynNode* type, size_t alignment = 0, EvalMode evalMode = modeUnspecified);
    
    DynNode* mkCtValue(const Location& loc, TypeRef typeNode, string data);
    DynNode* mkNull(const Location& loc, DynNode* typeNode);
    DynNode* mkStackAlloc(const Location& loc, DynNode* typeNode, int numElements = 1, int alignment = 0);
    DynNode* mkVarRef(const Location& loc, DynNode* varDecl);
    DynNode* mkFieldRef(const Location& loc, DynNode* obj, DynNode* fieldDecl);
    DynNode* mkFunRef(const Location& loc, DynNode* funDecl, DynNode* resType);
    DynNode* mkFunCall(const Location& loc, DynNode* funDecl, DynNodeVector args);
    DynNode* mkMemLoad(const Location& loc, DynNode* exp, size_t alignment = 0, bool isVolatile = false, AtomicOrdering ordering = atomicNone, bool singleThreaded = false);
    DynNode* mkMemStore(const Location& loc, DynNode* value, DynNode* address, size_t alignment = 0, bool isVolatile = false, AtomicOrdering ordering = atomicNone, bool singleThreaded = false);
    DynNode* mkBitcast(const Location& loc, DynNode* destType, DynNode* exp);
    DynNode* mkConditional(const Location& loc, DynNode* condition, DynNode* alt1, DynNode* alt2);
    
    DynNode* mkIf(const Location& loc, DynNode* condition, DynNode* thenClause, DynNode* elseClause, bool isCt = false);
    DynNode* mkWhile(const Location& loc, DynNode* condition, DynNode* body, DynNode* step, bool isCt = false);
    DynNode* mkBreak(const Location& loc);
    DynNode* mkContinue(const Location& loc);
    DynNode* mkReturn(const Location& loc, DynNode* exp = nullptr);
    
    template <typename T>
    DynNode* mkCtValue(const Location& loc, TypeRef type, T* dataVal)
    {
        const char* p = reinterpret_cast<const char*>(dataVal);
        string dataStr(p, p+sizeof(*dataVal));
        return mkCtValue(loc, type, move(dataStr));
    }
}
