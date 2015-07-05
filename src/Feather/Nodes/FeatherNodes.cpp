#include <StdInc.h>
#include "FeatherNodes.h"
#include "Nop.h"
#include "TypeNode.h"
#include "BackendCode.h"
#include "NodeList.h"
#include "LocalSpace.h"
#include "GlobalConstructAction.h"
#include "GlobalDestructAction.h"
#include "ScopeDestructAction.h"
#include "TempDestructAction.h"

#include "Decls/Function.h"
#include "Decls/Class.h"
#include "Decls/Var.h"

#include "Exp/CtValue.h"
#include "Exp/Null.h"
#include "Exp/StackAlloc.h"
#include "Exp/VarRef.h"
#include "Exp/FieldRef.h"
#include "Exp/FunRef.h"
#include "Exp/FunCall.h"
#include "Exp/MemLoad.h"
#include "Exp/MemStore.h"
#include "Exp/Bitcast.h"
#include "Exp/Conditional.h"

#include "Stmt/If.h"
#include "Stmt/While.h"
#include "Stmt/Break.h"
#include "Stmt/Continue.h"
#include "Stmt/Return.h"

#include <Util/Decl.h>

#include <Nest/Common/Diagnostic.h>


using namespace Feather;

#define REQUIRE_NODE(loc, node) \
    if ( node ) ; else \
        REP_INTERNAL((loc), "Expected AST node (%1%)") % ( #node )
#define REQUIRE_TYPE(loc, type) \
    if ( type ) ; else \
        REP_INTERNAL((loc), "Expected type (%1%)") % ( #type )

NodeList* Feather::mkNodeList(const Location& loc, NodeVector children, bool voidResult)
{
    return new NodeList(loc, move(children), voidResult);
}
NodeList* Feather::addToNodeList(NodeList* prevList, Node* element)
{
    if ( !prevList )
    {
        prevList = new NodeList(element ? element->location() : Location(), {}, true);
    }
    
    prevList->addChild(element);
    return prevList;
}
NodeList* Feather::appendNodeList(NodeList* list, NodeList* newNodes)
{
    if ( !list )
        return newNodes;
    if ( !newNodes )
        return list;
    
    list->children().insert(list->children().end(), newNodes->children().begin(), newNodes->children().end());
    return list;
}


Node* Feather::mkNop(const Location& loc)
{
    return new Nop(loc);
}
Node* Feather::mkTypeNode(const Location& loc, TypeRef type)
{
    return new TypeNode(loc, type);
}
Node* Feather::mkBackendCode(const Location& loc, string code, EvalMode evalMode)
{
    return new BackendCode(loc, move(code), evalMode);
}
Node* Feather::mkLocalSpace(const Location& loc, NodeVector children)
{
    return new LocalSpace(loc, move(children));
}
Node* Feather::mkGlobalConstructAction(const Location& loc, Node* action)
{
    REQUIRE_NODE(loc, action);
    return new GlobalConstructAction(loc, action);
}
Node* Feather::mkGlobalDestructAction(const Location& loc, Node* action)
{
    REQUIRE_NODE(loc, action);
    return new GlobalDestructAction(loc, action);
}
Node* Feather::mkScopeDestructAction(const Location& loc, Node* action)
{
    REQUIRE_NODE(loc, action);
    return new ScopeDestructAction(loc, action);
}
Node* Feather::mkTempDestructAction(const Location& loc, Node* action)
{
    REQUIRE_NODE(loc, action);
    return new TempDestructAction(loc, action);
}


Node* Feather::mkFunction(const Location& loc, string name, Node* resType, NodeVector params, Node* body, CallConvention callConv, EvalMode evalMode)
{
    Function* fun = new Function(loc, name, resType, body, move(params), callConv);
    setEvalMode(fun, evalMode);
    return fun;
}
Node* Feather::mkClass(const Location& loc, string name, NodeVector fields, EvalMode evalMode)
{
    Class* res = new Class(loc, name, move(fields));
    setEvalMode(res, evalMode);
    return res;
}
Node* Feather::mkVar(const Location& loc, string name, Node* typeNode, size_t alignment, EvalMode evalMode)
{
    REQUIRE_TYPE(loc, typeNode);
    Var* res = new Var(loc, move(name), typeNode, alignment);
    setEvalMode(res, evalMode);
    return res;
}


Node* Feather::mkCtValue(const Location& loc, TypeRef type, string data)
{
    REQUIRE_TYPE(loc, type);
    return new CtValue(loc, type, move(data));
}
Node* Feather::mkNull(const Location& loc, Node* typeNode)
{
    REQUIRE_NODE(loc, typeNode);
    return new Null(loc, typeNode);
}
Node* Feather::mkStackAlloc(const Location& loc, Node* typeNode, int numElements, int alignment)
{
    REQUIRE_NODE(loc, typeNode);
    return new StackAlloc(loc, typeNode, numElements, alignment);
}
Node* Feather::mkVarRef(const Location& loc, Node* varDecl)
{
    REQUIRE_NODE(loc, varDecl);
    if ( varDecl->nodeKind() != nkFeatherDeclVar )
        REP_INTERNAL(loc, "A VarRef node must be applied on a variable declaration (%1% given)") % varDecl->nodeKindName();
    return new VarRef(loc, varDecl);
}
Node* Feather::mkFieldRef(const Location& loc, Node* obj, Node* fieldDecl)
{
    REQUIRE_NODE(loc, obj);
    REQUIRE_NODE(loc, fieldDecl);
    if ( fieldDecl->nodeKind() != nkFeatherDeclVar )
        REP_INTERNAL(loc, "A FieldRef node must be applied on a field declaration (%1% given)") % fieldDecl->nodeKindName();
    return new FieldRef(loc, obj, fieldDecl);
}
Node* Feather::mkFunRef(const Location& loc, Node* funDecl, Node* resType)
{
    REQUIRE_NODE(loc, funDecl);
    REQUIRE_NODE(loc, resType);
    Function* fun = funDecl->as<Function>();
    if ( !fun )
        REP_INTERNAL(loc, "A FunRef node must be applied on a function declaration (%1% given)") % funDecl->nodeKindName();
    return new FunRef(loc, fun, resType);
}
Node* Feather::mkFunCall(const Location& loc, Node* funDecl, NodeVector args)
{
    REQUIRE_NODE(loc, funDecl);
    Function* fun = funDecl->as<Function>();
    if ( !fun )
        REP_INTERNAL(loc, "A FunCall node must be applied on a function declaration (%1% given)") % funDecl->nodeKindName();
    return new FunCall(loc, fun, args);
}
Node* Feather::mkMemLoad(const Location& loc, Node* exp, size_t alignment, bool isVolatile, AtomicOrdering ordering, bool singleThreaded)
{
    REQUIRE_NODE(loc, exp);
    return new MemLoad(loc, exp, alignment, isVolatile, ordering, singleThreaded);
}
Node* Feather::mkMemStore(const Location& loc, Node* value, Node* address, size_t alignment, bool isVolatile, AtomicOrdering ordering, bool singleThreaded)
{
    REQUIRE_NODE(loc, value);
    REQUIRE_NODE(loc, address);
    return new MemStore(loc, value, address, alignment, isVolatile, ordering, singleThreaded);
}
Node* Feather::mkBitcast(const Location& loc, Node* destType, Node* exp)
{
    REQUIRE_NODE(loc, destType);
    REQUIRE_NODE(loc, exp);
    return new Bitcast(loc, destType, exp);
}
Node* Feather::mkConditional(const Location& loc, Node* condition, Node* alt1, Node* alt2)
{
    REQUIRE_NODE(loc, condition);
    REQUIRE_NODE(loc, alt1);
    REQUIRE_NODE(loc, alt2);
    return new Conditional(loc, condition, alt1, alt2);
}


Node* Feather::mkIf(const Location& loc, Node* condition, Node* thenClause, Node* elseClause, bool isCt)
{
    REQUIRE_NODE(loc, condition);
    return new If(loc, condition, thenClause, elseClause);
}
Node* Feather::mkWhile(const Location& loc, Node* condition, Node* body, Node* step, bool isCt)
{
    REQUIRE_NODE(loc, condition);
    return new While(loc, condition, body, step, isCt);
}
Node* Feather::mkBreak(const Location& loc)
{
    return new Break(loc);
}
Node* Feather::mkContinue(const Location& loc)
{
    return new Continue(loc);
}
Node* Feather::mkReturn(const Location& loc, Node* exp)
{
    return new Return(loc, exp);
}
