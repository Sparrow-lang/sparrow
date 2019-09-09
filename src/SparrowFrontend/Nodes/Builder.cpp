#include <StdInc.h>
#include "Builder.h"
#include "Helpers/DeclsHelpers.h"
#include <NodeCommonsCpp.h>

#include "Feather/Api/Feather.h"
#include "Nest/Utils/cppif/NodeUtils.hpp"

using namespace SprFrontend;

namespace {
template <typename T> StringRef toStrData(T val) {
    const T* ptr = &val;
    const T* end = ptr + 1;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    StringRef res = {reinterpret_cast<const char*>(ptr), reinterpret_cast<const char*>(end)};
    return res.dup();
}
} // namespace

LocStringVec* SprFrontend::buildStringList(LocStringVec* prevList, LocString element) {
    if (!prevList)
        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
        prevList = new LocStringVec;

    prevList->emplace_back(move(element));
    return prevList;
}

Node* SprFrontend::buildSprFunctionExp(const Location& loc, StringRef name, Node* parameters,
        Node* returnType, Node* bodyExp, Node* ifClause) {
    const Location& loc2 = bodyExp->location;
    Node* body = buildBlockStmt(
            loc2, Feather_mkNodeList(loc2, fromIniList({mkReturnStmt(loc2, bodyExp)})));
    if (!returnType)
        returnType = mkFunApplication(loc2, mkIdentifier(loc2, StringRef("typeOf")),
                Feather_mkNodeList(loc2, fromIniList({bodyExp})));

    return mkSprFunction(loc, name, parameters, returnType, body, ifClause);
}

Node* SprFrontend::buildPostfixOp(const Location& loc, StringRef op, Node* base) {
    return mkOperatorCall(loc, base, move(op), nullptr);
}

Node* SprFrontend::buildPrefixOp(const Location& loc, StringRef op, Node* base) {
    return mkInfixOp(loc, move(op), nullptr, base);
}

Node* SprFrontend::buildParenthesisExp(const Location& loc, Node* exp) {
    return Feather_mkNodeList(loc, fromIniList({exp}));
}

Node* SprFrontend::buildIntLiteral(const Location& loc, int value) {
    return mkLiteral(loc, StringRef("Int"), toStrData(value));
}

Node* SprFrontend::buildUIntLiteral(const Location& loc, unsigned int value) {
    return mkLiteral(loc, StringRef("UInt32"), toStrData(value));
}

Node* SprFrontend::buildLongLiteral(const Location& loc, long value) {
    return mkLiteral(loc, StringRef("Int64"), toStrData(value));
}

Node* SprFrontend::buildULongLiteral(const Location& loc, unsigned long value) {
    return mkLiteral(loc, StringRef("UInt64"), toStrData(value));
}

Node* SprFrontend::buildFloatLiteral(const Location& loc, float value) {
    return mkLiteral(loc, StringRef("Float32"), toStrData(value));
}

Node* SprFrontend::buildDoubleLiteral(const Location& loc, double value) {
    return mkLiteral(loc, StringRef("Float"), toStrData(value));
}

Node* SprFrontend::buildCharLiteral(const Location& loc, char value) {
    return mkLiteral(loc, StringRef("Char"), toStrData(value));
}

Node* SprFrontend::buildStringLiteral(const Location& loc, StringRef value) {
    return mkLiteral(loc, StringRef("StringRef"), move(value));
}

Node* SprFrontend::buildNullLiteral(const Location& loc) {
    return mkLiteral(loc, StringRef("Null"), StringRef{});
}

Node* SprFrontend::buildBoolLiteral(const Location& loc, bool value) {
    return mkLiteral(loc, StringRef("Bool"), toStrData(value));
}
Node* SprFrontend::buildLiteral(const Location& loc, StringRef litType, void* value) {
    return mkLiteral(loc, litType, toStrData(value));
}

Node* SprFrontend::buildExpressionStmt(const Location& /*loc*/, Node* exp) { return exp; }

Node* SprFrontend::buildBlockStmt(const Location& loc, Node* statements) {
    return Feather_mkLocalSpace(loc, statements ? all(statements->children) : fromIniList({}));
}

Node* SprFrontend::buildIfStmt(
        const Location& loc, Node* cond, Node* thenClause, Node* elseClause) {
    return Feather_mkIf(loc, cond, thenClause, elseClause);
}

Node* SprFrontend::buildWhileStmt(const Location& loc, Node* cond, Node* step, Node* action) {
    return Feather_mkWhile(loc, cond, action, step);
}

Node* SprFrontend::buildBreakStmt(const Location& loc) { return Feather_mkBreak(loc); }

Node* SprFrontend::buildContinueStmt(const Location& loc) { return Feather_mkContinue(loc); }
