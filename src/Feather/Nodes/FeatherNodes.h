#pragma once

#include <Nest/Intermediate/Node.h>
#include <Nest/Intermediate/EvalMode.h>
#include <Feather/Nodes/Exp/AtomicOrdering.h>
#include <Feather/Nodes/Decls/CallConvention.h>

FWD_CLASS1(Feather, NodeList);

namespace Feather
{
    using namespace Nest;

    NodeList* mkNodeList(const Location& loc, NodeVector children, bool voidResult = false);
    NodeList* addToNodeList(NodeList* prevList, Node* element);
    NodeList* appendNodeList(NodeList* list, NodeList* newNodes);

    Node* mkNop(const Location& loc);
    Node* mkTypeNode(const Location& loc, TypeRef type);
    Node* mkBackendCode(const Location& loc, string code, EvalMode evalMode = modeRt);
    Node* mkLocalSpace(const Location& loc, NodeVector children);
    Node* mkGlobalConstructAction(const Location& loc, Node* action);
    Node* mkGlobalDestructAction(const Location& loc, Node* action);
    Node* mkScopeDestructAction(const Location& loc, Node* action);
    Node* mkTempDestructAction(const Location& loc, Node* action);
    
    Node* mkFunction(const Location& loc, string name, Node* resType, NodeVector params, Node* body, CallConvention callConv = ccC, EvalMode evalMode = modeUnspecified);
    Node* mkClass(const Location& loc, string name, NodeVector fields, EvalMode evalMode = modeUnspecified);
    Node* mkVar(const Location& loc, string name, Node* type, size_t alignment = 0, EvalMode evalMode = modeUnspecified);
    
    Node* mkCtValue(const Location& loc, TypeRef typeNode, string data);
    Node* mkNull(const Location& loc, Node* typeNode);
    Node* mkStackAlloc(const Location& loc, Node* typeNode, int numElements = 1, int alignment = 0);
    Node* mkVarRef(const Location& loc, Node* varDecl);
    Node* mkFieldRef(const Location& loc, Node* obj, Node* fieldDecl);
    Node* mkFunRef(const Location& loc, Node* funDecl, Node* resType);
    Node* mkFunCall(const Location& loc, Node* funDecl, NodeVector args);
    Node* mkMemLoad(const Location& loc, Node* exp, size_t alignment = 0, bool isVolatile = false, AtomicOrdering ordering = atomicNone, bool singleThreaded = false);
    Node* mkMemStore(const Location& loc, Node* value, Node* address, size_t alignment = 0, bool isVolatile = false, AtomicOrdering ordering = atomicNone, bool singleThreaded = false);
    Node* mkBitcast(const Location& loc, Node* destType, Node* exp);
    Node* mkConditional(const Location& loc, Node* condition, Node* alt1, Node* alt2);
    
    Node* mkIf(const Location& loc, Node* condition, Node* thenClause, Node* elseClause, bool isCt = false);
    Node* mkWhile(const Location& loc, Node* condition, Node* body, Node* step, bool isCt = false);
    Node* mkBreak(const Location& loc);
    Node* mkContinue(const Location& loc);
    Node* mkReturn(const Location& loc, Node* exp = nullptr);
    
    template <typename T>
    Node* mkCtValue(const Location& loc, TypeRef type, T* dataVal)
    {
        const char* p = reinterpret_cast<const char*>(dataVal);
        string dataStr(p, p+sizeof(*dataVal));
        return mkCtValue(loc, type, move(dataStr));
    }
}
