#pragma once

#include <Nest/Intermediate/DynNode.h>
#include <Nest/Intermediate/EvalMode.h>
#include <SparrowFrontend/Nodes/Decls/AccessType.h>

namespace SprFrontend
{
    using namespace Nest;
    
    DynNode* mkModifiers(const Location& loc, DynNode* main, DynNode* mods);

    DynNode* mkSprCompilationUnit(const Location& loc, DynNode* package, NodeList* imports, NodeList* declarations);
    DynNode* mkSprUsing(const Location& loc, string alias, DynNode* usingNode, AccessType accessType = publicAccess);
    DynNode* mkSprPackage(const Location& loc, string name, NodeList* children, AccessType accessType = publicAccess);
    DynNode* mkSprVariable(const Location& loc, string name, DynNode* typeNode, DynNode* init, AccessType accessType = publicAccess);
    DynNode* mkSprVariable(const Location& loc, string name, TypeRef type, DynNode* init, AccessType accessType = publicAccess);
    DynNode* mkSprClass(const Location& loc, string name, NodeList* parameters, NodeList* baseClasses, DynNode* ifClause, NodeList* children, AccessType accessType = publicAccess);

    DynNode* mkSprConcept(const Location& loc, string name, string paramName, DynNode* baseConcept, DynNode* ifClause, AccessType accessType = publicAccess);

    DynNode* mkSprFunction(const Location& loc, string name, NodeList* parameters, DynNode* returnType, DynNode* body, DynNode* ifClause = nullptr, AccessType accessType = publicAccess);
    DynNode* mkSprFunctionExp(const Location& loc, string name, NodeList* parameters, DynNode* returnType, DynNode* bodyExp, DynNode* ifClause = nullptr, AccessType accessType = publicAccess);
    DynNode* mkSprParameter(const Location& loc, string name, DynNode* typeNode, DynNode* init = nullptr);
    DynNode* mkSprParameter(const Location& loc, string name, TypeRef type, DynNode* init = nullptr);
    DynNode* mkSprAutoParameter(const Location& loc, string name);

    DynNode* mkIdentifier(const Location& loc, string id);
    DynNode* mkCompoundExp(const Location& loc, DynNode* base, string id);
    DynNode* mkStarExp(const Location& loc, DynNode* base, const string& operName);
    DynNode* mkPostfixOp(const Location& loc, string op, DynNode* base);
    DynNode* mkInfixOp(const Location& loc, string op, DynNode* arg1, DynNode* arg2);
    DynNode* mkPrefixOp(const Location& loc, string op, DynNode* base);
    DynNode* mkFunApplication(const Location& loc, DynNode* base, NodeList* arguments);
    DynNode* mkFunApplication(const Location& loc, DynNode* base, DynNodeVector arguments);
    DynNode* mkOperatorCall(const Location& loc, DynNode* arg1, string op, DynNode* arg2);

    DynNode* mkConditionalExp(const Location& loc, DynNode* cond, DynNode* alt1, DynNode* alt2);
    DynNode* mkThisExp(const Location& loc);
    DynNode* mkParenthesisExp(const Location& loc, DynNode* exp);
    DynNode* mkIntLiteral(const Location& loc, int value);
    DynNode* mkUIntLiteral(const Location& loc, unsigned int value);
    DynNode* mkLongLiteral(const Location& loc, long value);
    DynNode* mkULongLiteral(const Location& loc, unsigned long value);
    DynNode* mkFloatLiteral(const Location& loc, float value);
    DynNode* mkDoubleLiteral(const Location& loc, double value);
    DynNode* mkCharLiteral(const Location& loc, char value);
    DynNode* mkStringLiteral(const Location& loc, string value);
    DynNode* mkNullLiteral(const Location& loc);
    DynNode* mkBoolLiteral(const Location& loc, bool value);
    DynNode* mkLambdaExp(const Location& loc, NodeList* parameters, DynNode* returnType, DynNode* body, DynNode* bodyExp, NodeList* closureParams);
    
    DynNode* mkExpressionStmt(const Location& loc, DynNode* exp);
    DynNode* mkBlockStmt(const Location& loc, NodeList* statements);
    DynNode* mkIfStmt(const Location& loc, DynNode* cond, DynNode* thenClause, DynNode* elseClause);
    DynNode* mkForStmt(const Location& loc, string name, DynNode* type, DynNode* range, DynNode* action);
    DynNode* mkWhileStmt(const Location& loc, DynNode* cond, DynNode* step, DynNode* action);
    DynNode* mkBreakStmt(const Location& loc);
    DynNode* mkContinueStmt(const Location& loc);
    DynNode* mkReturnStmt(const Location& loc, DynNode* exp);
//    DynNode* mkThrowStmt(const Location& loc, DynNode* exp);
//    DynNode* mkTryStmt(const Location& loc, DynNode* actions, NodeList* catches, DynNode* finallyBlock);
//    DynNode* mkCatchBlock(const Location& loc, DynNode* formal, DynNode* action);
//    DynNode* mkFinallyBlock(const Location& loc, DynNode* action);

    DynNode* mkDeclExp(const Location& loc, DynNodeVector decls, DynNode* baseExp = nullptr);

    DynNode* mkInstantiation(const Location& loc, DynNodeVector boundValues, DynNodeVector boundVars);
    DynNode* mkInstantiationsSet(DynNode* parentNode, DynNodeVector params, DynNode* ifClause);
}
