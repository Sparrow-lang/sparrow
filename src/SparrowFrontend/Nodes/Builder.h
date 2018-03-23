#pragma once

#include <NodeCommonsH.h>

#include <SparrowFrontend/Nodes/SparrowNodes.h>

#include "Decls/AccessType.h"

namespace SprFrontend {
typedef pair<Location, string> LocString;
typedef vector<LocString> LocStringVec;

Node* addModifiers(Node* mods, Node* main);

LocStringVec* buildStringList(LocStringVec* prevList, LocString element);

Node* buildSprFunctionExp(const Location& loc, StringRef name, Node* parameters, Node* returnType,
        Node* bodyExp, Node* ifClause = nullptr);

Node* buildPostfixOp(const Location& loc, StringRef op, Node* base);
Node* buildPrefixOp(const Location& loc, StringRef op, Node* base);
Node* buildParenthesisExp(const Location& loc, Node* exp);
Node* buildIntLiteral(const Location& loc, int value);
Node* buildUIntLiteral(const Location& loc, unsigned int value);
Node* buildLongLiteral(const Location& loc, long value);
Node* buildULongLiteral(const Location& loc, unsigned long value);
Node* buildFloatLiteral(const Location& loc, float value);
Node* buildDoubleLiteral(const Location& loc, double value);
Node* buildCharLiteral(const Location& loc, char value);
Node* buildStringLiteral(const Location& loc, StringRef value);
Node* buildNullLiteral(const Location& loc);
Node* buildBoolLiteral(const Location& loc, bool value);
Node* buildLiteral(const Location& loc, StringRef litType, void* value);

Node* buildExpressionStmt(const Location& loc, Node* exp);
Node* buildBlockStmt(const Location& loc, Node* statements);
Node* buildIfStmt(const Location& loc, Node* cond, Node* thenClause, Node* elseClause);
Node* buildWhileStmt(const Location& loc, Node* cond, Node* step, Node* action);
Node* buildBreakStmt(const Location& loc);
Node* buildContinueStmt(const Location& loc);
} // namespace SprFrontend
