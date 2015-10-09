#include <StdInc.h>
#include "Builder.h"
#include <NodeCommonsCpp.h>

#include <Feather/Nodes/FeatherNodes.h>
#include <Nest/Intermediate/NodeVector.h>


using namespace SprFrontend;

namespace
{
    template <typename T>
    StringRef toStrData(T val)
    {
        const T* ptr = &val;
        const T* end = ptr + 1;
        StringRef res = { reinterpret_cast<const char*>(ptr), reinterpret_cast<const char*>(end) };
        return dup(res);
    }
}


vector<string>* SprFrontend::buildStringList(vector<string>* prevList, string element)
{
    if ( !prevList )
        prevList = new vector<string>;

    prevList->emplace_back(move(element));
    return prevList;
}

Node* SprFrontend::buildVariables(const Location& loc, const vector<string>& names, Node* typeNode, Node* init, Node* mods, AccessType accessType)
{
    NodeVector nodes;
    nodes.reserve(names.size());
    for ( const string& name: names )
    {
        nodes.push_back(mkModifiers(loc, mkSprVariable(loc, fromString(name), typeNode, init, accessType), mods));
    }
    return Feather::mkNodeList(loc, all(nodes), true);
}

Node* SprFrontend::buildParameters(const Location& loc, const vector<string>& names, Node* typeNode, Node* init, Node* mods)
{
    NodeVector nodes;
    nodes.reserve(names.size());
    for ( const string& name: names )
    {
        nodes.push_back(mkModifiers(loc, mkSprParameter(loc, fromString(name), typeNode, init), mods));
    }
    return Feather::mkNodeList(loc, all(nodes), true);
}

Node* SprFrontend::buildAutoParameter(const Location& loc, StringRef name, Node* mods)
{
    Node* typeNode = mkIdentifier(loc, fromCStr("AnyType"));
    Node* param = mkModifiers(loc, mkSprParameter(loc, name, typeNode, nullptr), mods);
    return Feather::mkNodeList(loc, fromIniList({param}), true);
}

Node* SprFrontend::buildSprFunctionExp(const Location& loc, StringRef name, Node* parameters, Node* returnType, Node* bodyExp, Node* ifClause, AccessType accessType)
{
    const Location& loc2 = bodyExp->location;
    Node* body = buildBlockStmt(loc2, Feather::mkNodeList(loc2, fromIniList({ mkReturnStmt(loc2, bodyExp) })));
    if ( !returnType )
        returnType = mkFunApplication(loc2, mkIdentifier(loc2, fromCStr("typeOf")), Feather::mkNodeList(loc2, fromIniList({ bodyExp })));

    return mkSprFunction(loc, name, parameters, returnType, body, ifClause, accessType);
}


Node* SprFrontend::buildPostfixOp(const Location& loc, StringRef op, Node* base)
{
    return mkOperatorCall(loc, base, move(op), nullptr);
}

Node* SprFrontend::buildPrefixOp(const Location& loc, StringRef op, Node* base)
{
    return mkInfixOp(loc, move(op), nullptr, base);
}

Node* SprFrontend::buildParenthesisExp(const Location& loc, Node* exp)
{
    return Feather::mkNodeList(loc, fromIniList({exp}), false);
}

Node* SprFrontend::buildIntLiteral(const Location& loc, int value)
{
    return mkLiteral(loc, fromCStr("Int"), toStrData(value));
}

Node* SprFrontend::buildUIntLiteral(const Location& loc, unsigned int value)
{
    return mkLiteral(loc, fromCStr("UInt"), toStrData(value));
}

Node* SprFrontend::buildLongLiteral(const Location& loc, long value)
{
    return mkLiteral(loc, fromCStr("Long"), toStrData(value));
}

Node* SprFrontend::buildULongLiteral(const Location& loc, unsigned long value)
{
    return mkLiteral(loc, fromCStr("ULong"), toStrData(value));
}

Node* SprFrontend::buildFloatLiteral(const Location& loc, float value)
{
    return mkLiteral(loc, fromCStr("Float"), toStrData(value));
}

Node* SprFrontend::buildDoubleLiteral(const Location& loc, double value)
{
    return mkLiteral(loc, fromCStr("Double"), toStrData(value));
}

Node* SprFrontend::buildCharLiteral(const Location& loc, char value)
{
    return mkLiteral(loc, fromCStr("Char"), toStrData(value));
}

Node* SprFrontend::buildStringLiteral(const Location& loc, StringRef value)
{
    return mkLiteral(loc, fromCStr("StringRef"), move(value));
}

Node* SprFrontend::buildNullLiteral(const Location& loc)
{
    return mkLiteral(loc, fromCStr("Null"), allocStringRef(0));
}

Node* SprFrontend::buildBoolLiteral(const Location& loc, bool value)
{
    return mkLiteral(loc, fromCStr("Bool"), toStrData(value));
}


Node* SprFrontend::buildExpressionStmt(const Location& /*loc*/, Node* exp)
{
    return exp;
}

Node* SprFrontend::buildBlockStmt(const Location& loc, Node* statements)
{
    return Feather::mkLocalSpace(loc, statements ? all(statements->children) : fromIniList({}));
}

Node* SprFrontend::buildIfStmt(const Location& loc, Node* cond, Node* thenClause, Node* elseClause)
{
    return Feather::mkIf(loc, cond, thenClause, elseClause);
}

Node* SprFrontend::buildWhileStmt(const Location& loc, Node* cond, Node* step, Node* action)
{
    return Feather::mkWhile(loc, cond, action, step);
}

Node* SprFrontend::buildBreakStmt(const Location& loc)
{
    return Feather::mkBreak(loc);
}

Node* SprFrontend::buildContinueStmt(const Location& loc)
{
    return Feather::mkContinue(loc);
}


