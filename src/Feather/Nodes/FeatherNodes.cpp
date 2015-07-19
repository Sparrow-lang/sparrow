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
#include "FeatherNodeCommonsCpp.h"

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

#include <Feather/FeatherTypes.h>

#include <Util/Decl.h>
#include <Util/TypeTraits.h>

#include <Nest/Common/Diagnostic.h>
#include <Nest/Intermediate/Modifier.h>


using namespace Feather;

#define REQUIRE_NODE(loc, node) \
    if ( node ) ; else \
        REP_INTERNAL((loc), "Expected AST node (%1%)") % ( #node )
#define REQUIRE_TYPE(loc, type) \
    if ( type ) ; else \
        REP_INTERNAL((loc), "Expected type (%1%)") % ( #type )

namespace
{
    class CtProcessMod : public Nest::Modifier
    {
    public:
        virtual void afterSemanticCheck(Node* node)
        {
            theCompiler().ctProcess(node);
        };
    };

    const char* propResultVoid = "nodeList.resultVoid";


    Node* Nop_SemanticCheck(Node* node)
    {
        node->type = getVoidType(node->context->evalMode());
        return node;
    }

    Node* TypeNode_SemanticCheck(Node* node)
    {
        node->type = Nest::getCheckPropertyType(node, "givenType");
        return node;
    }
    const char* TypeNode_toString(const Node* node)
    {
        ostringstream os;
        os << "type(" << getCheckPropertyType(node, "givenType") << ")";
        return strdup(os.str().c_str());
    }

    Node* BackendCode_SemanticCheck(Node* node)
    {
        EvalMode mode = BackendCode_getEvalMode(node);
        if ( !node->type )
            node->type = getVoidType(mode);

        if ( mode != modeRt )
        {
            // CT process this node right after semantic check
            addModifier(node, new CtProcessMod);
        }
        return node;
    }
    const char* BackendCode_toString(const Node* node)
    {
        ostringstream os;
        os << "backendCode(" << BackendCode_getCode(node) << ")";
        return strdup(os.str().c_str());
    }

    TypeRef NodeList_ComputeType(Node* node)
    {
        // Compute the type for all the children
        for ( Node* c: node->children )
        {
            if ( c )
                Nest::computeType(c);
        }

        // Get the type of the last node
        TypeRef res = ( hasProperty(node, propResultVoid) || node->children.empty() || !node->children.back()->type ) ? getVoidType(node->context->evalMode()) : node->children.back()->type;
        res = adjustMode(res, node->context, node->location);
        return res;
    }
    Node* NodeList_SemanticCheck(Node* node)
    {
        // Semantic check each of the children
        bool hasNonCtChildren = false;
        for ( Node* c: node->children )
        {
            if ( c )
            {
                Nest::semanticCheck(c);
                hasNonCtChildren = hasNonCtChildren || !isCt(c);
            }
        }

        // Make sure the type is computed
        if ( !node->type )
        {
            // Get the type of the last node
            TypeRef t = ( hasProperty(node, propResultVoid) || node->children.empty() || !node->children.back()->type ) ? getVoidType(node->context->evalMode()) : node->children.back()->type;
            t = adjustMode(t, node->context, node->location);
            node->type = t;
            checkEvalMode(node);
        }
        return node;
    }

    void LocalSpace_SetContextForChildren(Node* node)
    {
        node->childrenContext = node->context->createChildContext(node);
        Nest::defaultFunSetContextForChildren(node);
    }
    TypeRef LocalSpace_ComputeType(Node* node)
    {
        return getVoidType(node->context->evalMode());
    }
    Node* LocalSpace_SemanticCheck(Node* node)
    {
        // Compute type first
        computeType(node);

        // Semantic check each of the children
        for ( Node* c: node->children )
        {
            try
            {
                Nest::semanticCheck(c);
            }
            catch(...)
            {
                // Don't pass errors upwards
            }
        }
        checkEvalMode(node);
        return node;
    }

    Node* GlobalConstructAction_SemanticCheck(Node* node)
    {
        Nest::semanticCheck(node->children[0]);
        node->type = getVoidType(node->context->evalMode());

        // For CT construct actions, evaluate them asap
        if ( isCt(node->children[0]) )
        {
            theCompiler().ctEval(node->children[0]);
            return mkNop(node->location)->node();
        }
        return node;
    }

    Node* GlobalDestructAction_SemanticCheck(Node* node)
    {
        Nest::semanticCheck(node->children[0]);
        node->type = getVoidType(node->context->evalMode());

        // We never CT evaluate global destruct actions
        if ( isCt(node->children[0]) )
        {
            return mkNop(node->location)->node();
        }
        return node;
    }

    // Both for ScopeDestructAction and for TempDestructAction
    Node* ScopeTempDestructAction_SemanticCheck(Node* node)
    {
        Nest::semanticCheck(node->children[0]);
        node->type = getVoidType(node->context->evalMode());
        return node;
    }
}

int Feather::firstFeatherNodeKind = 0;

int Feather::nkFeatherNop = 0;
int Feather::nkFeatherTypeNode = 0;
int Feather::nkFeatherBackendCode = 0;
int Feather::nkFeatherNodeList = 0;
int Feather::nkFeatherLocalSpace = 0;
int Feather::nkFeatherGlobalConstructAction = 0;
int Feather::nkFeatherGlobalDestructAction = 0;
int Feather::nkFeatherScopeDestructAction = 0;
int Feather::nkFeatherTempDestructAction = 0;

int Feather::nkFeatherDeclFunction = 0;
int Feather::nkFeatherDeclClass = 0;
int Feather::nkFeatherDeclVar = 0;

int Feather::nkFeatherExpCtValue = 0;
int Feather::nkFeatherExpNull = 0;
int Feather::nkFeatherExpStackAlloc = 0;
int Feather::nkFeatherExpVarRef = 0;
int Feather::nkFeatherExpFieldRef = 0;
int Feather::nkFeatherExpFunRef = 0;
int Feather::nkFeatherExpFunCall = 0;
int Feather::nkFeatherExpMemLoad = 0;
int Feather::nkFeatherExpMemStore = 0;
int Feather::nkFeatherExpBitcast = 0;
int Feather::nkFeatherExpConditional = 0;
int Feather::nkFeatherExpChangeMode = 0;

int Feather::nkFeatherStmtIf = 0;
int Feather::nkFeatherStmtWhile = 0;
int Feather::nkFeatherStmtBreak = 0;
int Feather::nkFeatherStmtContinue = 0;
int Feather::nkFeatherStmtReturn = 0;

void Feather::initFeatherNodeKinds()
{
    nkFeatherNop = registerNodeKind("nop", &Nop_SemanticCheck);
    nkFeatherTypeNode = registerNodeKind("typeNode", &TypeNode_SemanticCheck, NULL, NULL, &TypeNode_toString);
    nkFeatherBackendCode = registerNodeKind("backendCode", &BackendCode_SemanticCheck, NULL, NULL, &BackendCode_toString);
    nkFeatherNodeList = registerNodeKind("nodeList", &NodeList_SemanticCheck, &NodeList_ComputeType, NULL, NULL);
    nkFeatherLocalSpace = registerNodeKind("localSpace", &LocalSpace_SemanticCheck, &LocalSpace_ComputeType, &LocalSpace_SetContextForChildren, NULL);
    nkFeatherGlobalConstructAction = registerNodeKind("globalConstructAction", &GlobalConstructAction_SemanticCheck, NULL, NULL, NULL);
    nkFeatherGlobalDestructAction = registerNodeKind("globalDestructAction", &GlobalDestructAction_SemanticCheck, NULL, NULL, NULL);
    nkFeatherScopeDestructAction = registerNodeKind("scopelDestructAction", &ScopeTempDestructAction_SemanticCheck, NULL, NULL, NULL);
    nkFeatherTempDestructAction = registerNodeKind("tempDestructAction", &ScopeTempDestructAction_SemanticCheck, NULL, NULL, NULL);

    Nop::classNodeKindRef() = nkFeatherNop;
    TypeNode::classNodeKindRef() = nkFeatherTypeNode;
    BackendCode::classNodeKindRef() = nkFeatherBackendCode;
    NodeList::classNodeKindRef() = nkFeatherNodeList;
    LocalSpace::classNodeKindRef() = nkFeatherLocalSpace;
    GlobalConstructAction::classNodeKindRef() = nkFeatherGlobalConstructAction;
    GlobalDestructAction::classNodeKindRef() = nkFeatherGlobalDestructAction;
    ScopeDestructAction::classNodeKindRef() = nkFeatherScopeDestructAction;
    TempDestructAction::classNodeKindRef() = nkFeatherTempDestructAction;
    
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

    nkFeatherDeclFunction =             Function::classNodeKind();
    nkFeatherDeclClass =                Class::classNodeKind();
    nkFeatherDeclVar =                  Var::classNodeKind();
    
    nkFeatherExpCtValue =               CtValue::classNodeKind();
    nkFeatherExpNull =                  Null::classNodeKind();
    nkFeatherExpStackAlloc =            StackAlloc::classNodeKind();
    nkFeatherExpVarRef =                VarRef::classNodeKind();
    nkFeatherExpFieldRef =              FieldRef::classNodeKind();
    nkFeatherExpFunRef =                FunRef::classNodeKind();
    nkFeatherExpFunCall =               FunCall::classNodeKind();
    nkFeatherExpMemLoad =               MemLoad::classNodeKind();
    nkFeatherExpMemStore =              MemStore::classNodeKind();
    nkFeatherExpBitcast =               Bitcast::classNodeKind();
    nkFeatherExpConditional =           Conditional::classNodeKind();
    nkFeatherExpChangeMode =            ChangeMode::classNodeKind();
    
    nkFeatherStmtIf =                   If::classNodeKind();
    nkFeatherStmtWhile =                While::classNodeKind();
    nkFeatherStmtBreak =                Break::classNodeKind();
    nkFeatherStmtContinue =             Continue::classNodeKind();
    nkFeatherStmtReturn =               Return::classNodeKind();

    firstFeatherNodeKind = nkFeatherNop;
}


NodeList* Feather::mkNodeList(const Location& loc, DynNodeVector children, bool voidResult)
{
    Node* res = createNode(nkFeatherNodeList);
    res->location = loc;
    if ( voidResult )
        setProperty(res, propResultVoid, 1);
    res->children = move(fromDyn(children));
    return (NodeList*) res;
}
NodeList* Feather::addToNodeList(NodeList* prevList, DynNode* element)
{
    Node* res = prevList->node();
    if ( !res )
    {
        res = createNode(nkFeatherNodeList);
        if ( element )
            res->location = element->location();
        setProperty(res, propResultVoid, 1);    // voidResult == true
    }
    
    res->children.push_back(element->node());
    return (NodeList*) res;
}
NodeList* Feather::appendNodeList(NodeList* list, NodeList* newNodes)
{
    if ( !list )
        return newNodes;
    if ( !newNodes )
        return list;
    
    Node* res = list->node();
    NodeVector& otherChildren = newNodes->node()->children;
    res->children.insert(res->children.end(), otherChildren.begin(), otherChildren.end());
    return (NodeList*) res;
}


DynNode* Feather::mkNop(const Location& loc)
{
    Node* res = createNode(nkFeatherNop);
    res->location = loc;
    return (DynNode*) res;
}
DynNode* Feather::mkTypeNode(const Location& loc, TypeRef type)
{
    Node* res = createNode(nkFeatherTypeNode);
    res->location = loc;
    setProperty(res, "givenType", type);
    return (DynNode*) res;
}
DynNode* Feather::mkBackendCode(const Location& loc, string code, EvalMode evalMode)
{
    Node* res = createNode(nkFeatherBackendCode);
    res->location = loc;
    setProperty(res, propCode, move(code));
    setProperty(res, propEvalMode, (int) evalMode);
    return (DynNode*) res;
}
DynNode* Feather::mkLocalSpace(const Location& loc, DynNodeVector children)
{
    Node* res = createNode(nkFeatherLocalSpace);
    res->location = loc;
    res->children = move(fromDyn(children));
    return (DynNode*) res;
}
DynNode* Feather::mkGlobalConstructAction(const Location& loc, DynNode* action)
{
    REQUIRE_NODE(loc, action);
    Node* res = createNode(nkFeatherGlobalConstructAction);
    res->location = loc;
    res->children = { action->node() };
    return (DynNode*) res;
}
DynNode* Feather::mkGlobalDestructAction(const Location& loc, DynNode* action)
{
    REQUIRE_NODE(loc, action);
    Node* res = createNode(nkFeatherGlobalDestructAction);
    res->location = loc;
    res->children = { action->node() };
    return (DynNode*) res;
}
DynNode* Feather::mkScopeDestructAction(const Location& loc, DynNode* action)
{
    REQUIRE_NODE(loc, action);
    Node* res = createNode(nkFeatherScopeDestructAction);
    res->location = loc;
    res->children = { action->node() };
    return (DynNode*) res;
}
DynNode* Feather::mkTempDestructAction(const Location& loc, DynNode* action)
{
    REQUIRE_NODE(loc, action);
    Node* res = createNode(nkFeatherTempDestructAction);
    res->location = loc;
    res->children = { action->node() };
    return (DynNode*) res;
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


const char* Feather::BackendCode_getCode(const Node* node)
{
    ASSERT(node->nodeKind == nkFeatherBackendCode);
    return getCheckPropertyString(node, propCode).c_str();
}

EvalMode Feather::BackendCode_getEvalMode(Node* node)
{
    ASSERT(node->nodeKind == nkFeatherBackendCode);
    EvalMode curMode = (EvalMode) getCheckPropertyInt(node, propEvalMode);
    return curMode != modeUnspecified ? curMode : node->context->evalMode();
}
