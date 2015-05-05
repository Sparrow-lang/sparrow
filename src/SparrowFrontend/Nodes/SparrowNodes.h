#pragma once

#include <Nest/Intermediate/Node.h>
#include <Nest/Intermediate/EvalMode.h>
#include <SparrowFrontend/Nodes/Decls/AccessType.h>

namespace SprFrontend
{
    using namespace Nest;
    
    Node* mkModifiers(const Location& loc, Node* main, Node* mods);

    Node* mkSprCompilationUnit(const Location& loc, Node* package, NodeList* imports, NodeList* declarations);
    Node* mkSprUsing(const Location& loc, string alias, Node* usingNode, AccessType accessType = publicAccess);
    Node* mkSprPackage(const Location& loc, string name, NodeList* children, AccessType accessType = publicAccess);
    Node* mkSprVariable(const Location& loc, string name, Node* typeNode, Node* init, AccessType accessType = publicAccess);
    Node* mkSprVariable(const Location& loc, string name, Type* type, Node* init, AccessType accessType = publicAccess);
    Node* mkSprClass(const Location& loc, string name, NodeList* parameters, NodeList* baseClasses, Node* ifClause, NodeList* children, AccessType accessType = publicAccess);

    Node* mkSprConcept(const Location& loc, string name, string paramName, Node* baseConcept, Node* ifClause, AccessType accessType = publicAccess);

    Node* mkSprFunction(const Location& loc, string name, NodeList* parameters, Node* returnType, Node* body, Node* ifClause = nullptr, AccessType accessType = publicAccess);
    Node* mkSprFunctionExp(const Location& loc, string name, NodeList* parameters, Node* returnType, Node* bodyExp, Node* ifClause = nullptr, AccessType accessType = publicAccess);
    Node* mkSprParameter(const Location& loc, string name, Node* typeNode, Node* init = nullptr);
    Node* mkSprParameter(const Location& loc, string name, Type* type, Node* init = nullptr);
    Node* mkSprAutoParameter(const Location& loc, string name);

    Node* mkIdentifier(const Location& loc, string id);
    Node* mkCompoundExp(const Location& loc, Node* base, string id);
    Node* mkStarExp(const Location& loc, Node* base, const string& operName);
    Node* mkPostfixOp(const Location& loc, string op, Node* base);
    Node* mkInfixOp(const Location& loc, string op, Node* arg1, Node* arg2);
    Node* mkPrefixOp(const Location& loc, string op, Node* base);
    Node* mkFunApplication(const Location& loc, Node* base, NodeList* arguments);
    Node* mkFunApplication(const Location& loc, Node* base, NodeVector arguments);
    Node* mkOperatorCall(const Location& loc, Node* arg1, string op, Node* arg2);

    Node* mkConditionalExp(const Location& loc, Node* cond, Node* alt1, Node* alt2);
    Node* mkThisExp(const Location& loc);
    Node* mkParenthesisExp(const Location& loc, Node* exp);
    Node* mkIntLiteral(const Location& loc, int value);
    Node* mkUIntLiteral(const Location& loc, unsigned int value);
    Node* mkLongLiteral(const Location& loc, long value);
    Node* mkULongLiteral(const Location& loc, unsigned long value);
    Node* mkFloatLiteral(const Location& loc, float value);
    Node* mkDoubleLiteral(const Location& loc, double value);
    Node* mkCharLiteral(const Location& loc, char value);
    Node* mkStringLiteral(const Location& loc, string value);
    Node* mkNullLiteral(const Location& loc);
    Node* mkBoolLiteral(const Location& loc, bool value);
    Node* mkLambdaExp(const Location& loc, NodeList* parameters, Node* returnType, Node* body, Node* bodyExp, NodeList* closureParams);
    
    Node* mkExpressionStmt(const Location& loc, Node* exp);
    Node* mkBlockStmt(const Location& loc, NodeList* statements);
    Node* mkIfStmt(const Location& loc, Node* cond, Node* thenClause, Node* elseClause);
    Node* mkForStmt(const Location& loc, string name, Node* type, Node* range, Node* action);
    Node* mkWhileStmt(const Location& loc, Node* cond, Node* step, Node* action);
    Node* mkBreakStmt(const Location& loc);
    Node* mkContinueStmt(const Location& loc);
    Node* mkReturnStmt(const Location& loc, Node* exp);
//    Node* mkThrowStmt(const Location& loc, Node* exp);
//    Node* mkTryStmt(const Location& loc, Node* actions, NodeList* catches, Node* finallyBlock);
//    Node* mkCatchBlock(const Location& loc, Node* formal, Node* action);
//    Node* mkFinallyBlock(const Location& loc, Node* action);

    Node* mkDeclExp(const Location& loc, NodeVector decls, Node* baseExp = nullptr);

    Node* mkInstantiation(const Location& loc, NodeVector boundValues, NodeVector boundVars);
    Node* mkInstantiationsSet(Node* parentNode, NodeVector params, Node* ifClause);
}
