#include "Feather/src/StdInc.h"
#include "Feather/src/Api/Feather_Nodes.h"
#include "Feather/src/FeatherNodeCommonsCpp.h"

#include "Feather/Api/Feather.h"

#include "Feather/Utils/cppif/FeatherNodes.hpp"

#include "Nest/Utils/cppif/StringRef.hpp"
#include "Nest/Utils/cppif/NodeRange.hpp"

using namespace Feather;

int _firstFeatherNodeKind = 0;

int nkFeatherNop = 0;
int nkFeatherTypeNode = 0;
int nkFeatherBackendCode = 0;
int nkFeatherNodeList = 0;
int nkFeatherLocalSpace = 0;
int nkFeatherGlobalConstructAction = 0;
int nkFeatherGlobalDestructAction = 0;
int nkFeatherScopeDestructAction = 0;
int nkFeatherTempDestructAction = 0;
int nkFeatherChangeMode = 0;

int nkFeatherDeclFunction = 0;
int nkFeatherDeclClass = 0;
int nkFeatherDeclVar = 0;

int nkFeatherExpCtValue = 0;
int nkFeatherExpNull = 0;
int nkFeatherExpVarRef = 0;
int nkFeatherExpFieldRef = 0;
int nkFeatherExpFunRef = 0;
int nkFeatherExpFunCall = 0;
int nkFeatherExpMemLoad = 0;
int nkFeatherExpMemStore = 0;
int nkFeatherExpBitcast = 0;
int nkFeatherExpConditional = 0;

int nkFeatherStmtIf = 0;
int nkFeatherStmtWhile = 0;
int nkFeatherStmtBreak = 0;
int nkFeatherStmtContinue = 0;
int nkFeatherStmtReturn = 0;

void initFeatherNodeKinds() {
    nkFeatherNop = Nop::registerNodeKind();
    nkFeatherTypeNode = TypeNode::registerNodeKind();
    nkFeatherBackendCode = BackendCode::registerNodeKind();
    nkFeatherNodeList = NodeList::registerNodeKind();
    nkFeatherLocalSpace = LocalSpace::registerNodeKind();
    nkFeatherGlobalConstructAction = GlobalConstructAction::registerNodeKind();
    nkFeatherGlobalDestructAction = GlobalDestructAction::registerNodeKind();
    nkFeatherScopeDestructAction = ScopeDestructAction::registerNodeKind();
    nkFeatherTempDestructAction = TempDestructAction::registerNodeKind();
    nkFeatherChangeMode = ChangeMode::registerNodeKind();

    nkFeatherDeclFunction = FunctionDecl::registerNodeKind();
    nkFeatherDeclClass = StructDecl::registerNodeKind();
    nkFeatherDeclVar = VarDecl::registerNodeKind();

    nkFeatherExpCtValue = CtValueExp::registerNodeKind();
    nkFeatherExpNull = NullExp::registerNodeKind();
    nkFeatherExpVarRef = VarRefExp::registerNodeKind();
    nkFeatherExpFieldRef = FieldRefExp::registerNodeKind();
    nkFeatherExpFunRef = FunRefExp::registerNodeKind();
    nkFeatherExpFunCall = FunCallExp::registerNodeKind();
    nkFeatherExpMemLoad = MemLoadExp::registerNodeKind();
    nkFeatherExpMemStore = MemStoreExp::registerNodeKind();
    nkFeatherExpBitcast = BitcastExp::registerNodeKind();
    nkFeatherExpConditional = ConditionalExp::registerNodeKind();

    nkFeatherStmtIf = IfStmt::registerNodeKind();
    nkFeatherStmtWhile = WhileStmt::registerNodeKind();
    nkFeatherStmtBreak = BreakStmt::registerNodeKind();
    nkFeatherStmtContinue = ContinueStmt::registerNodeKind();
    nkFeatherStmtReturn = ReturnStmt::registerNodeKind();

    _firstFeatherNodeKind = nkFeatherNop;
}

int Feather_getFirstFeatherNodeKind() { return _firstFeatherNodeKind; }

Node* Feather_mkNodeList(Location loc, Nest_NodeRange children) {
    return NodeList::create(loc, children);
}
Node* Feather_mkNodeListVoid(Location loc, Nest_NodeRange children) {
    return NodeList::create(loc, children, true);
}
Node* Feather_addToNodeList(Node* prevList, Node* element) {
    return NodeList::append(NodeList(prevList), NodeHandle(element));
}
Node* Feather_appendNodeList(Node* list, Node* newNodes) {
    return NodeList::append(NodeList(list), NodeList(newNodes));
}

Node* Feather_mkNop(Location loc) { return Nop::create(loc); }
Node* Feather_mkTypeNode(Location loc, TypeRef type) { return TypeNode::create(loc, type); }
Node* Feather_mkBackendCode(Location loc, Nest_StringRef code, EvalMode evalMode) {
    return BackendCode::create(loc, code, evalMode);
}
Node* Feather_mkLocalSpace(Location loc, Nest_NodeRange children) {
    return LocalSpace::create(loc, children);
}
Node* Feather_mkGlobalConstructAction(Location loc, Node* action) {
    return GlobalConstructAction::create(loc, action);
}
Node* Feather_mkGlobalDestructAction(Location loc, Node* action) {
    return GlobalDestructAction::create(loc, action);
}
Node* Feather_mkScopeDestructAction(Location loc, Node* action) {
    return ScopeDestructAction::create(loc, action);
}
Node* Feather_mkTempDestructAction(Location loc, Node* action) {
    return TempDestructAction::create(loc, action);
}
Node* Feather_mkChangeMode(Location loc, Node* child, EvalMode mode) {
    return ChangeMode::create(loc, child, mode);
}

Node* Feather_mkFunction(
        Location loc, Nest_StringRef name, Node* resType, Nest_NodeRange params, Node* body) {
    return FunctionDecl::create(loc, name, resType, params, body);
}
Node* Feather_mkClass(Location loc, Nest_StringRef name, Nest_NodeRange fields) {
    return StructDecl::create(loc, name, fields);
}
Node* Feather_mkVar(Location loc, Nest_StringRef name, Node* typeNode) {
    return VarDecl::create(loc, name, typeNode);
}

Node* Feather_mkCtValue(Location loc, TypeRef type, Nest_StringRef data) {
    return CtValueExp::create(loc, type, data);
}
Node* Feather_mkNull(Location loc, Node* typeNode) { return NullExp::create(loc, typeNode); }
Node* Feather_mkVarRef(Location loc, Node* varDecl) { return VarRefExp::create(loc, varDecl); }
Node* Feather_mkFieldRef(Location loc, Node* obj, Node* fieldDecl) {
    return FieldRefExp::create(loc, obj, fieldDecl);
}
Node* Feather_mkFunRef(Location loc, Node* funDecl, Node* resType) {
    return FunRefExp::create(loc, funDecl, resType);
}
Node* Feather_mkFunCall(Location loc, Node* funDecl, Nest_NodeRange args) {
    return FunCallExp::create(loc, funDecl, args);
}
Node* Feather_mkMemLoad(Location loc, Node* exp) { return MemLoadExp::create(loc, exp); }
Node* Feather_mkMemStore(Location loc, Node* value, Node* address) {
    return MemStoreExp::create(loc, value, address);
}
Node* Feather_mkBitcast(Location loc, Node* destType, Node* exp) {
    return BitcastExp::create(loc, destType, exp);
}
Node* Feather_mkConditional(Location loc, Node* condition, Node* alt1, Node* alt2) {
    return ConditionalExp::create(loc, condition, alt1, alt2);
}

Node* Feather_mkIf(Location loc, Node* condition, Node* thenClause, Node* elseClause) {
    return IfStmt::create(loc, condition, thenClause, elseClause);
}
Node* Feather_mkWhile(Location loc, Node* condition, Node* body, Node* step) {
    return WhileStmt::create(loc, condition, body, step);
}
Node* Feather_mkBreak(Location loc) { return BreakStmt::create(loc); }
Node* Feather_mkContinue(Location loc) { return ContinueStmt::create(loc); }
Node* Feather_mkReturn(Location loc, Node* exp) { return ReturnStmt::create(loc, exp); }
