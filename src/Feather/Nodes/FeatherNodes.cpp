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
#include "ChangeMode.h"

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

void Feather::initFeatherNodeKinds()
{
    Feather::Nop::registerSelf();
    Feather::TypeNode::registerSelf();
    Feather::BackendCode::registerSelf();
    Feather::NodeList::registerSelf();
    Feather::LocalSpace::registerSelf();
    Feather::GlobalConstructAction::registerSelf();
    Feather::GlobalDestructAction::registerSelf();
    Feather::ScopeDestructAction::registerSelf();
    Feather::TempDestructAction::registerSelf();
    
    Feather::Function::registerSelf();
    Feather::Class::registerSelf();
    Feather::Var::registerSelf();
    
    Feather::CtValue::registerSelf();
    Feather::Null::registerSelf();
    Feather::StackAlloc::registerSelf();
    Feather::VarRef::registerSelf();
    Feather::FieldRef::registerSelf();
    Feather::FunRef::registerSelf();
    Feather::FunCall::registerSelf();
    Feather::MemLoad::registerSelf();
    Feather::MemStore::registerSelf();
    Feather::Bitcast::registerSelf();
    Feather::Conditional::registerSelf();
    Feather::ChangeMode::registerSelf();
    
    Feather::If::registerSelf();
    Feather::While::registerSelf();
    Feather::Break::registerSelf();
    Feather::Continue::registerSelf();
    Feather::Return::registerSelf();
}


NodeList* Feather::mkNodeList(const Location& loc, DynNodeVector children, bool voidResult)
{
    return new NodeList(loc, move(children), voidResult);
}
NodeList* Feather::addToNodeList(NodeList* prevList, DynNode* element)
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


DynNode* Feather::mkNop(const Location& loc)
{
    return new Nop(loc);
}
DynNode* Feather::mkTypeNode(const Location& loc, TypeRef type)
{
    return new TypeNode(loc, type);
}
DynNode* Feather::mkBackendCode(const Location& loc, string code, EvalMode evalMode)
{
    return new BackendCode(loc, move(code), evalMode);
}
DynNode* Feather::mkLocalSpace(const Location& loc, DynNodeVector children)
{
    return new LocalSpace(loc, move(children));
}
DynNode* Feather::mkGlobalConstructAction(const Location& loc, DynNode* action)
{
    REQUIRE_NODE(loc, action);
    return new GlobalConstructAction(loc, action);
}
DynNode* Feather::mkGlobalDestructAction(const Location& loc, DynNode* action)
{
    REQUIRE_NODE(loc, action);
    return new GlobalDestructAction(loc, action);
}
DynNode* Feather::mkScopeDestructAction(const Location& loc, DynNode* action)
{
    REQUIRE_NODE(loc, action);
    return new ScopeDestructAction(loc, action);
}
DynNode* Feather::mkTempDestructAction(const Location& loc, DynNode* action)
{
    REQUIRE_NODE(loc, action);
    return new TempDestructAction(loc, action);
}


DynNode* Feather::mkFunction(const Location& loc, string name, DynNode* resType, DynNodeVector params, DynNode* body, CallConvention callConv, EvalMode evalMode)
{
    Function* fun = new Function(loc, name, resType, body, move(params), callConv);
    setEvalMode(fun->node(), evalMode);
    return fun;
}
DynNode* Feather::mkClass(const Location& loc, string name, DynNodeVector fields, EvalMode evalMode)
{
    Class* res = new Class(loc, name, move(fields));
    setEvalMode(res->node(), evalMode);
    return res;
}
DynNode* Feather::mkVar(const Location& loc, string name, DynNode* typeNode, size_t alignment, EvalMode evalMode)
{
    REQUIRE_TYPE(loc, typeNode);
    Var* res = new Var(loc, move(name), typeNode, alignment);
    setEvalMode(res->node(), evalMode);
    return res;
}


DynNode* Feather::mkCtValue(const Location& loc, TypeRef type, string data)
{
    REQUIRE_TYPE(loc, type);
    return new CtValue(loc, type, move(data));
}
DynNode* Feather::mkNull(const Location& loc, DynNode* typeNode)
{
    REQUIRE_NODE(loc, typeNode);
    return new Null(loc, typeNode);
}
DynNode* Feather::mkStackAlloc(const Location& loc, DynNode* typeNode, int numElements, int alignment)
{
    REQUIRE_NODE(loc, typeNode);
    return new StackAlloc(loc, typeNode, numElements, alignment);
}
DynNode* Feather::mkVarRef(const Location& loc, DynNode* varDecl)
{
    REQUIRE_NODE(loc, varDecl);
    if ( varDecl->nodeKind() != nkFeatherDeclVar )
        REP_INTERNAL(loc, "A VarRef node must be applied on a variable declaration (%1% given)") % varDecl->nodeKindName();
    return new VarRef(loc, varDecl);
}
DynNode* Feather::mkFieldRef(const Location& loc, DynNode* obj, DynNode* fieldDecl)
{
    REQUIRE_NODE(loc, obj);
    REQUIRE_NODE(loc, fieldDecl);
    if ( fieldDecl->nodeKind() != nkFeatherDeclVar )
        REP_INTERNAL(loc, "A FieldRef node must be applied on a field declaration (%1% given)") % fieldDecl->nodeKindName();
    return new FieldRef(loc, obj, fieldDecl);
}
DynNode* Feather::mkFunRef(const Location& loc, DynNode* funDecl, DynNode* resType)
{
    REQUIRE_NODE(loc, funDecl);
    REQUIRE_NODE(loc, resType);
    Function* fun = funDecl->as<Function>();
    if ( !fun )
        REP_INTERNAL(loc, "A FunRef node must be applied on a function declaration (%1% given)") % funDecl->nodeKindName();
    return new FunRef(loc, fun, resType);
}
DynNode* Feather::mkFunCall(const Location& loc, DynNode* funDecl, DynNodeVector args)
{
    REQUIRE_NODE(loc, funDecl);
    Function* fun = funDecl->as<Function>();
    if ( !fun )
        REP_INTERNAL(loc, "A FunCall node must be applied on a function declaration (%1% given)") % funDecl->nodeKindName();
    return new FunCall(loc, fun, args);
}
DynNode* Feather::mkMemLoad(const Location& loc, DynNode* exp, size_t alignment, bool isVolatile, AtomicOrdering ordering, bool singleThreaded)
{
    REQUIRE_NODE(loc, exp);
    return new MemLoad(loc, exp, alignment, isVolatile, ordering, singleThreaded);
}
DynNode* Feather::mkMemStore(const Location& loc, DynNode* value, DynNode* address, size_t alignment, bool isVolatile, AtomicOrdering ordering, bool singleThreaded)
{
    REQUIRE_NODE(loc, value);
    REQUIRE_NODE(loc, address);
    return new MemStore(loc, value, address, alignment, isVolatile, ordering, singleThreaded);
}
DynNode* Feather::mkBitcast(const Location& loc, DynNode* destType, DynNode* exp)
{
    REQUIRE_NODE(loc, destType);
    REQUIRE_NODE(loc, exp);
    return new Bitcast(loc, destType, exp);
}
DynNode* Feather::mkConditional(const Location& loc, DynNode* condition, DynNode* alt1, DynNode* alt2)
{
    REQUIRE_NODE(loc, condition);
    REQUIRE_NODE(loc, alt1);
    REQUIRE_NODE(loc, alt2);
    return new Conditional(loc, condition, alt1, alt2);
}


DynNode* Feather::mkIf(const Location& loc, DynNode* condition, DynNode* thenClause, DynNode* elseClause, bool isCt)
{
    REQUIRE_NODE(loc, condition);
    return new If(loc, condition, thenClause, elseClause);
}
DynNode* Feather::mkWhile(const Location& loc, DynNode* condition, DynNode* body, DynNode* step, bool isCt)
{
    REQUIRE_NODE(loc, condition);
    return new While(loc, condition, body, step, isCt);
}
DynNode* Feather::mkBreak(const Location& loc)
{
    return new Break(loc);
}
DynNode* Feather::mkContinue(const Location& loc)
{
    return new Continue(loc);
}
DynNode* Feather::mkReturn(const Location& loc, DynNode* exp)
{
    return new Return(loc, exp);
}
