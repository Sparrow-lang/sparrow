#include "Feather/src/StdInc.h"
#include "Feather/Utils/cppif/FeatherNodes.hpp"
#include "Feather/src/FeatherNodeCommonsCpp.h"
#include "Feather/Api/Feather.h"
#include "Feather/Utils/FeatherUtils.h"

#include "Nest/Utils/Diagnostic.hpp"
#include "Nest/Utils/cppif/NodeRange.hpp"
#include "Nest/Utils/cppif/StringRef.hpp"

#define REQUIRE_NODE(loc, node)                                                                    \
    if (node)                                                                                      \
        ;                                                                                          \
    else                                                                                           \
        REP_INTERNAL((loc), "Expected AST node (%1%)") % (#node)

#define REQUIRE_NODE_KIND(node, kind)                                                              \
    if (node && node->nodeKind != nkFeatherNop)                                                    \
        REP_INTERNAL(NOLOC, "Expected AST node of kind %1%, found %2% (%3%)") % (#kind) % node %   \
                __FUNCTION__;                                                                      \
    else                                                                                           \
        ;

namespace Feather {

namespace {

//! Helper function used to create nodes
template <typename T> T createNode(int kind, const Nest::Location& loc) {
    T res;
    res.handle = Nest::NodeHandle::create(kind, loc).handle;
    return res;
}

const char* propResultVoid = "nodeList.resultVoid";

} // namespace

Nop Nop::create(const Nest::Location& loc) { return createNode<Nop>(nkFeatherNop, loc); }
Nop::Nop(Nest::Node* n)
    : NodeHandle(n) {
    REQUIRE_NODE_KIND(n, nkFeatherNop);
}

TypeNode TypeNode::create(const Nest::Location& loc, TypeBase type) {
    TypeNode res = createNode<TypeNode>(nkFeatherTypeNode, loc);
    res.setProperty("givenType", type);
    return res;
}
TypeNode::TypeNode(Nest::Node* n)
    : NodeHandle(n) {
    REQUIRE_NODE_KIND(n, nkFeatherTypeNode);
}
TypeBase TypeNode::givenType() const { return getCheckPropertyType("givenType"); }

BackendCode BackendCode::create(const Nest::Location& loc, Nest::StringRef code, EvalMode mode) {
    auto res = createNode<BackendCode>(nkFeatherBackendCode, loc);
    res.setProperty(propCode, code);
    res.setProperty(propEvalMode, (int)mode);
    return res;
}
BackendCode::BackendCode(Nest::Node* n)
    : NodeHandle(n) {
    REQUIRE_NODE_KIND(n, nkFeatherBackendCode);
}
StringRef BackendCode::code() const { return getCheckPropertyString(propCode); }
EvalMode BackendCode::mode() const { return (EvalMode)getCheckPropertyInt(propEvalMode); }

NodeList NodeList::create(const Nest::Location& loc, Nest::NodeRange children, bool setVoid) {
    NodeList res = createNode<NodeList>(nkFeatherNodeList, loc);
    res.setChildren(children);
    if (setVoid)
        res.setProperty(propResultVoid, 1);
    return res;
}
NodeList NodeList::append(NodeList prev, NodeHandle node) {
    NodeList res = prev;
    if (!res) {
        Nest::Location loc = node ? node.location() : Nest::Location{};
        res = create(loc, Nest::NodeRange{}, true);
    }
    res.addChild(node);
    return res;
}
NodeList NodeList::append(NodeList prev, NodeList newNodes) {
    if (!prev)
        return newNodes;
    if (!newNodes)
        return prev;

    prev.addChildren(newNodes.children());
    return prev;
}
NodeList::NodeList(Nest::Node* n)
    : NodeHandle(n) {
    REQUIRE_NODE_KIND(n, nkFeatherNodeList);
}
bool NodeList::returnsVoid() const { return hasProperty(propResultVoid); }

LocalSpace LocalSpace::create(const Nest::Location& loc, Nest::NodeRange children) {
    LocalSpace res = createNode<LocalSpace>(nkFeatherLocalSpace, loc);
    res.setChildren(children);
    return res;
}
LocalSpace::LocalSpace(Nest::Node* n)
    : NodeHandle(n) {
    REQUIRE_NODE_KIND(n, nkFeatherLocalSpace);
}
GlobalConstructAction GlobalConstructAction::create(const Nest::Location& loc, NodeHandle action) {
    GlobalConstructAction res =
            createNode<GlobalConstructAction>(nkFeatherGlobalConstructAction, loc);
    res.setChildren(NodeRange{action});
    return res;
}
GlobalConstructAction::GlobalConstructAction(Nest::Node* n)
    : NodeHandle(n) {
    REQUIRE_NODE_KIND(n, nkFeatherGlobalConstructAction);
}
NodeHandle GlobalConstructAction::action() const { return children()[0]; }

GlobalDestructAction GlobalDestructAction::create(const Nest::Location& loc, NodeHandle action) {
    GlobalDestructAction res = createNode<GlobalDestructAction>(nkFeatherGlobalDestructAction, loc);
    res.setChildren(NodeRange{action});
    return res;
}
GlobalDestructAction::GlobalDestructAction(Nest::Node* n)
    : NodeHandle(n) {
    REQUIRE_NODE_KIND(n, nkFeatherGlobalDestructAction);
}
NodeHandle GlobalDestructAction::action() const { return children()[0]; }

ScopeDestructAction ScopeDestructAction::create(const Nest::Location& loc, NodeHandle action) {
    ScopeDestructAction res = createNode<ScopeDestructAction>(nkFeatherScopeDestructAction, loc);
    res.setChildren(NodeRange{action});
    return res;
}
ScopeDestructAction::ScopeDestructAction(Nest::Node* n)
    : NodeHandle(n) {
    REQUIRE_NODE_KIND(n, nkFeatherScopeDestructAction);
}
NodeHandle ScopeDestructAction::action() const { return children()[0]; }

TempDestructAction TempDestructAction::create(const Nest::Location& loc, NodeHandle action) {
    TempDestructAction res = createNode<TempDestructAction>(nkFeatherTempDestructAction, loc);
    res.setChildren(NodeRange{action});
    return res;
}
TempDestructAction::TempDestructAction(Nest::Node* n)
    : NodeHandle(n) {
    REQUIRE_NODE_KIND(n, nkFeatherTempDestructAction);
}
NodeHandle TempDestructAction::action() const { return children()[0]; }

ChangeMode ChangeMode::create(const Nest::Location& loc, NodeHandle child, EvalMode mode) {
    ChangeMode res = createNode<ChangeMode>(nkFeatherChangeMode, loc);
    res.setChildren(NodeRange{child});
    res.setProperty(propEvalMode, (int)mode);
    return res;
}
ChangeMode::ChangeMode(Nest::Node* n)
    : NodeHandle(n) {
    REQUIRE_NODE_KIND(n, nkFeatherChangeMode);
}
NodeHandle ChangeMode::child() const { return children()[0]; }
EvalMode ChangeMode::mode() const { return (EvalMode)getCheckPropertyInt(propEvalMode); }

DeclNode::DeclNode(Nest::Node* n)
    : NodeHandle(n) {}
StringRef DeclNode::name() const { return getCheckPropertyString("name"); }
EvalMode DeclNode::mode() const { return (EvalMode)getCheckPropertyInt(propEvalMode); }
void DeclNode::setNameAndMode(Nest::StringRef name, EvalMode mode) {
    setProperty("name", name);
    setProperty(propEvalMode, modeUnspecified);
}

FunctionDecl FunctionDecl::create(const Nest::Location& loc, StringRef name, NodeHandle resType,
        NodeRange params, NodeHandle body) {
    FunctionDecl res = createNode<FunctionDecl>(nkFeatherDeclFunction, loc);
    res.setChildren(NodeRange{resType, body});
    res.addChildren(params);
    res.setNameAndMode(name, modeUnspecified);
    res.setProperty("callConvention", (int)ccC);

    // Make sure all the nodes given as parameters have the right kind
    for (NodeHandle param : params) {
        if (param.explanation().kind() != nkFeatherDeclVar)
            REP_INTERNAL(param.location(), "Node %1% must be a parameter") % param;
    }

    return res;
}
FunctionDecl::FunctionDecl(Nest::Node* n)
    : DeclNode(n) {
    REQUIRE_NODE_KIND(n, nkFeatherDeclFunction);
}
NodeHandle FunctionDecl::resType() const { return children()[0]; }
NodeRange FunctionDecl::params() const { return children().skip(2); }
NodeHandle FunctionDecl::body() const { return children()[1]; }

StructDecl StructDecl::create(const Nest::Location& loc, StringRef name, NodeRange fields) {
    StructDecl res = createNode<StructDecl>(nkFeatherDeclClass, loc);
    res.setChildren(fields);
    res.setNameAndMode(name, modeUnspecified);

    // Make sure all the nodes given as fields have the right kind
    for (NodeHandle field : fields) {
        if (field.explanation().kind() != nkFeatherDeclVar)
            REP_INTERNAL(field.location(), "Node %1% must be a field") % field;
    }

    return res;
}
StructDecl::StructDecl(Nest::Node* n)
    : DeclNode(n) {
    REQUIRE_NODE_KIND(n, nkFeatherDeclClass);
}
NodeRange StructDecl::fields() const { return children(); }

VarDecl VarDecl::create(const Nest::Location& loc, StringRef name, NodeHandle typeNode) {
    VarDecl res = createNode<VarDecl>(nkFeatherDeclVar, loc);
    res.setChildren(NodeRange{typeNode});
    res.setNameAndMode(name, modeUnspecified);
    res.setProperty("alignment", 0);
    return res;
}
VarDecl::VarDecl(Nest::Node* n)
    : DeclNode(n) {
    REQUIRE_NODE_KIND(n, nkFeatherDeclVar);
}
NodeHandle VarDecl::typeNode() const { return children()[0]; }

CtValueExp CtValueExp::create(const Nest::Location& loc, TypeBase type, Nest::StringRef data) {
    CtValueExp res = createNode<CtValueExp>(nkFeatherExpCtValue, loc);
    res.setProperty("valueType", type);
    res.setProperty("valueData", data);
    return res;
}
CtValueExp::CtValueExp(Nest::Node* n)
    : NodeHandle(n) {
    REQUIRE_NODE_KIND(n, nkFeatherExpCtValue);
}
TypeBase CtValueExp::valueType() const { return getCheckPropertyType("valueType"); }
StringRef CtValueExp::valueData() const { return getCheckPropertyString("valueData"); }

NullExp NullExp::create(const Nest::Location& loc, NodeHandle typeNode) {
    NullExp res = createNode<NullExp>(nkFeatherExpNull, loc);
    res.setChildren(NodeRange{typeNode});
    return res;
}
NullExp::NullExp(Nest::Node* n)
    : NodeHandle(n) {
    REQUIRE_NODE_KIND(n, nkFeatherExpNull);
}
NodeHandle NullExp::typeNode() const { return children()[0]; }

VarRefExp VarRefExp::create(const Nest::Location& loc, VarDecl varDecl) {
    REQUIRE_NODE(loc, varDecl);

    VarRefExp res = createNode<VarRefExp>(nkFeatherExpVarRef, loc);
    res.setReferredNodes(NodeRange{varDecl});
    return res;
}
VarRefExp::VarRefExp(Nest::Node* n)
    : NodeHandle(n) {
    REQUIRE_NODE_KIND(n, nkFeatherExpVarRef);
}
VarDecl VarRefExp::varDecl() const { return VarDecl(referredNodes()[0]); }

FieldRefExp FieldRefExp::create(const Nest::Location& loc, NodeHandle obj, VarDecl fieldDecl) {
    REQUIRE_NODE(loc, obj);
    REQUIRE_NODE(loc, fieldDecl);
    FieldRefExp res = createNode<FieldRefExp>(nkFeatherExpFieldRef, loc);
    res.setChildren(NodeRange{obj});
    res.setReferredNodes(NodeRange{fieldDecl});
    return res;
}
FieldRefExp::FieldRefExp(Nest::Node* n)
    : NodeHandle(n) {
    REQUIRE_NODE_KIND(n, nkFeatherExpFieldRef);
}
NodeHandle FieldRefExp::object() const { return children()[0]; }
VarDecl FieldRefExp::fieldDecl() const { return VarDecl(referredNodes()[0]); }

FunRefExp FunRefExp::create(const Nest::Location& loc, FunctionDecl funDecl, NodeHandle resType) {
    REQUIRE_NODE(loc, funDecl);
    REQUIRE_NODE(loc, resType);
    FunRefExp res = createNode<FunRefExp>(nkFeatherExpFunRef, loc);
    res.setChildren(NodeRange{resType});
    res.setReferredNodes(NodeRange{funDecl});
    return res;
}
FunRefExp::FunRefExp(Nest::Node* n)
    : NodeHandle(n) {
    REQUIRE_NODE_KIND(n, nkFeatherExpFunRef);
}
FunctionDecl FunRefExp::funDecl() const { return FunctionDecl(referredNodes()[0]); }
NodeHandle FunRefExp::resTypeNode() const { return children()[0]; }

FunCallExp FunCallExp::create(const Nest::Location& loc, FunctionDecl funDecl, NodeRange args) {
    REQUIRE_NODE(loc, funDecl);
    FunCallExp res = createNode<FunCallExp>(nkFeatherExpFunCall, loc);
    res.setChildren(args);
    res.setReferredNodes(NodeRange{funDecl});
    return res;
}
FunCallExp::FunCallExp(Nest::Node* n)
    : NodeHandle(n) {
    REQUIRE_NODE_KIND(n, nkFeatherExpFunCall);
}
FunctionDecl FunCallExp::funDecl() const { return FunctionDecl(referredNodes()[0]); }
NodeRange FunCallExp::arguments() const { return children(); }

MemLoadExp MemLoadExp::create(const Nest::Location& loc, NodeHandle address) {
    REQUIRE_NODE(loc, address);
    MemLoadExp res = createNode<MemLoadExp>(nkFeatherExpMemLoad, loc);
    res.setChildren(NodeRange{address});
    return res;
}
MemLoadExp::MemLoadExp(Nest::Node* n)
    : NodeHandle(n) {
    REQUIRE_NODE_KIND(n, nkFeatherExpMemLoad);
}
NodeHandle MemLoadExp::address() const { return children()[0]; }

MemStoreExp MemStoreExp::create(const Nest::Location& loc, NodeHandle value, NodeHandle address) {
    REQUIRE_NODE(loc, value);
    REQUIRE_NODE(loc, address);
    MemStoreExp res = createNode<MemStoreExp>(nkFeatherExpMemStore, loc);
    res.setChildren(NodeRange{value, address});
    return res;
}
MemStoreExp::MemStoreExp(Nest::Node* n)
    : NodeHandle(n) {
    REQUIRE_NODE_KIND(n, nkFeatherExpMemStore);
}
NodeHandle MemStoreExp::value() const { return children()[0]; }
NodeHandle MemStoreExp::address() const { return children()[1]; }

BitcastExp BitcastExp::create(const Nest::Location& loc, NodeHandle destType, NodeHandle exp) {
    REQUIRE_NODE(loc, destType);
    REQUIRE_NODE(loc, exp);
    BitcastExp res = createNode<BitcastExp>(nkFeatherExpBitcast, loc);
    res.setChildren(NodeRange{destType, exp});
    return res;
}
BitcastExp::BitcastExp(Nest::Node* n)
    : NodeHandle(n) {
    REQUIRE_NODE_KIND(n, nkFeatherExpBitcast);
}
NodeHandle BitcastExp::destTypeNode() const { return children()[0]; }
NodeHandle BitcastExp::expression() const { return children()[1]; }

ConditionalExp ConditionalExp::create(
        const Nest::Location& loc, NodeHandle cond, NodeHandle alt1, NodeHandle alt2) {
    REQUIRE_NODE(loc, cond);
    REQUIRE_NODE(loc, alt1);
    REQUIRE_NODE(loc, alt2);
    ConditionalExp res = createNode<ConditionalExp>(nkFeatherExpConditional, loc);
    res.setChildren(NodeRange{cond, alt1, alt2});
    return res;
}
ConditionalExp::ConditionalExp(Nest::Node* n)
    : NodeHandle(n) {
    REQUIRE_NODE_KIND(n, nkFeatherExpConditional);
}
NodeHandle ConditionalExp::condition() const { return children()[0]; }
NodeHandle ConditionalExp::alt1() const { return children()[1]; }
NodeHandle ConditionalExp::alt2() const { return children()[2]; }

IfStmt IfStmt::create(
        const Nest::Location& loc, NodeHandle cond, NodeHandle thenC, NodeHandle elseC) {
    REQUIRE_NODE(loc, cond);
    REQUIRE_NODE(loc, thenC);
    REQUIRE_NODE(loc, elseC);
    IfStmt res = createNode<IfStmt>(nkFeatherStmtIf, loc);
    res.setChildren(NodeRange{cond, thenC, elseC});
    return res;
}
IfStmt::IfStmt(Nest::Node* n)
    : NodeHandle(n) {
    REQUIRE_NODE_KIND(n, nkFeatherStmtIf);
}
NodeHandle IfStmt::condition() const { return children()[0]; }
NodeHandle IfStmt::thenClause() const { return children()[1]; }
NodeHandle IfStmt::elseClause() const { return children()[2]; }

WhileStmt WhileStmt::create(
        const Nest::Location& loc, NodeHandle cond, NodeHandle body, NodeHandle step) {
    REQUIRE_NODE(loc, cond);
    REQUIRE_NODE(loc, body);
    WhileStmt res = createNode<WhileStmt>(nkFeatherStmtWhile, loc);
    res.setChildren(NodeRange{cond, body, step});
    return res;
}
WhileStmt::WhileStmt(Nest::Node* n)
    : NodeHandle(n) {
    REQUIRE_NODE_KIND(n, nkFeatherStmtWhile);
}
NodeHandle WhileStmt::condition() const { return children()[0]; }
NodeHandle WhileStmt::body() const { return children()[1]; }
NodeHandle WhileStmt::step() const { return children()[2]; }

BreakStmt BreakStmt::create(const Nest::Location& loc) {
    BreakStmt res = createNode<BreakStmt>(nkFeatherStmtBreak, loc);
    return res;
}
BreakStmt::BreakStmt(Nest::Node* n)
    : NodeHandle(n) {
    REQUIRE_NODE_KIND(n, nkFeatherStmtBreak);
}

ContinueStmt ContinueStmt::create(const Nest::Location& loc) {
    ContinueStmt res = createNode<ContinueStmt>(nkFeatherStmtContinue, loc);
    return res;
}
ContinueStmt::ContinueStmt(Nest::Node* n)
    : NodeHandle(n) {
    REQUIRE_NODE_KIND(n, nkFeatherStmtContinue);
}

ReturnStmt ReturnStmt::create(const Nest::Location& loc, NodeHandle exp) {
    ReturnStmt res = createNode<ReturnStmt>(nkFeatherStmtReturn, loc);
    res.setChildren(NodeRange{exp});
    return res;
}
ReturnStmt::ReturnStmt(Nest::Node* n)
    : NodeHandle(n) {
    REQUIRE_NODE_KIND(n, nkFeatherStmtReturn);
}
NodeHandle ReturnStmt::expression() const { return children()[0]; }

} // namespace Feather
