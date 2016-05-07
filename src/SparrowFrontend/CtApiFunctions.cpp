#include <StdInc.h>
#include "CtApiFunctions.h"
#include "UserDefinedSourceCode.h"
#include "SparrowSourceCode.h"

#include <Nodes/SparrowNodes.h>
#include <Nodes/Builder.h>

#include "Nest/Api/Backend.h"
#include "Nest/Api/SourceCode.h"

using namespace SprFrontend;
using namespace Nest;

namespace
{
    void ctApi_Sparrow_mkModifiers(Node** sret, Location* loc, Node* main, Node* mods)
    {
        *sret = mkModifiers(*loc, main, mods);
    }

    void ctApi_Sparrow_mkSprCompilationUnit(Node** sret, Location* loc, Node* package, Node* imports, Node* declarations)
    {
        *sret = mkSprCompilationUnit(*loc, package, imports, declarations);
    }
    void ctApi_Sparrow_mkSprUsing(Node** sret, Location* loc, StringRef alias, Node* usingNode)
    {
        *sret = mkSprUsing(*loc, alias, usingNode);
    }
    void ctApi_Sparrow_mkSprPackage(Node** sret, Location* loc, StringRef name, Node* children)
    {
        *sret = mkSprPackage(*loc, name, children);
    }
    void ctApi_Sparrow_mkSprVariable(Node** sret, Location* loc, StringRef name, Node* typeNode, Node* init)
    {
        *sret = mkSprVariable(*loc, name, typeNode, init);
    }
    void ctApi_Sparrow_mkSprClass(Node** sret, Location* loc, StringRef name, Node* parameters, Node* baseClasses, Node* ifClause, Node* children, int accessType)
    {
        *sret = mkSprClass(*loc, name, parameters, baseClasses, ifClause, children, (AccessType) accessType);
    }

    void ctApi_Sparrow_mkSprConcept(Node** sret, Location* loc, StringRef name, Node* baseConcept, StringRef paramName, Node* ifClause)
    {
        *sret = mkSprConcept(*loc, name, paramName, baseConcept, ifClause);
    }

    void ctApi_Sparrow_mkSprFunction(Node** sret, Location* loc, StringRef name, Node* parameters, Node* returnType, Node* body, Node* ifClause, int accessType)
    {
        *sret = mkSprFunction(*loc, name, parameters, returnType, body, nullptr, (AccessType) accessType);
    }
    void ctApi_Sparrow_mkSprFunctionExp(Node** sret, Location* loc, StringRef name, Node* parameters, Node* returnType, Node* bodyExp, Node* ifClause, int accessType)
    {
        *sret = buildSprFunctionExp(*loc, name, parameters, returnType, bodyExp, nullptr, (AccessType) accessType);
    }
    void ctApi_Sparrow_mkSprParameter(Node** sret, Location* loc, StringRef name, Node* typeNode, Node* init)
    {
        *sret = mkSprParameter(*loc, name, typeNode);
    }
    void ctApi_Sparrow_mkSprAutoParameter(Node** sret, Location* loc, StringRef name)
    {
        *sret = mkSprAutoParameter(*loc, name);
    }

    void ctApi_Sparrow_mkIdentifier(Node** sret, Location* loc, StringRef id)
    {
        *sret = mkIdentifier(*loc, id);
    }
    void ctApi_Sparrow_mkCompoundExp(Node** sret, Location* loc, Node* base, StringRef id)
    {
        *sret = mkCompoundExp(*loc, base, id);
    }
    void ctApi_Sparrow_mkStarExp(Node** sret, Location* loc, Node* base, StringRef operName)
    {
        *sret = mkStarExp(*loc, base, operName);
    }
    void ctApi_Sparrow_mkPostfixOp(Node** sret, Location* loc, StringRef op, Node* base)
    {
        *sret = buildPostfixOp(*loc, op, base);
    }
    void ctApi_Sparrow_mkInfixOp(Node** sret, Location* loc, StringRef op, Node* arg1, Node* arg2)
    {
        *sret = mkInfixOp(*loc, op, arg1, arg2);
    }
    void ctApi_Sparrow_mkPrefixOp(Node** sret, Location* loc, StringRef op, Node* base)
    {
        *sret = buildPrefixOp(*loc, op, base);
    }
    void ctApi_Sparrow_mkFunApplication(Node** sret, Location* loc, Node* base, Node* arguments)
    {
        *sret = mkFunApplication(*loc, base, arguments);
    }
    void ctApi_Sparrow_mkOperatorCall(Node** sret, Location* loc, Node* arg1, StringRef op, Node* arg2)
    {
        *sret = mkOperatorCall(*loc, arg1, op, arg2);
    }

    void ctApi_Sparrow_mkConditionalExp(Node** sret, Location* loc, Node* cond, Node* alt1, Node* alt2)
    {
        *sret = mkConditionalExp(*loc, cond, alt1, alt2);
    }
    void ctApi_Sparrow_mkThisExp(Node** sret, Location* loc)
    {
        *sret = mkThisExp(*loc);
    }
    void ctApi_Sparrow_mkParenthesisExp(Node** sret, Location* loc, Node* exp)
    {
        *sret = buildParenthesisExp(*loc, exp);
    }
    void ctApi_Sparrow_mkIntLiteral(Node** sret, Location* loc, int value)
    {
        *sret = buildIntLiteral(*loc, value);
    }
    void ctApi_Sparrow_mkUIntLiteral(Node** sret, Location* loc, unsigned int value)
    {
        *sret = buildUIntLiteral(*loc, value);
    }
    void ctApi_Sparrow_mkLongLiteral(Node** sret, Location* loc, long value)
    {
        *sret = buildLongLiteral(*loc, value);
    }
    void ctApi_Sparrow_mkULongLiteral(Node** sret, Location* loc, unsigned long value)
    {
        *sret = buildULongLiteral(*loc, value);
    }
    void ctApi_Sparrow_mkFloatLiteral(Node** sret, Location* loc, float value)
    {
        *sret = buildFloatLiteral(*loc, value);
    }
    void ctApi_Sparrow_mkDoubleLiteral(Node** sret, Location* loc, double value)
    {
        *sret = buildDoubleLiteral(*loc, value);
    }
    void ctApi_Sparrow_mkCharLiteral(Node** sret, Location* loc, char value)
    {
        *sret = buildCharLiteral(*loc, value);
    }
    void ctApi_Sparrow_mkStringLiteral(Node** sret, Location* loc, StringRef value)
    {
        *sret = buildStringLiteral(*loc, value);
    }
    void ctApi_Sparrow_mkNullLiteral(Node** sret, Location* loc)
    {
        *sret = buildNullLiteral(*loc);
    }
    void ctApi_Sparrow_mkBoolLiteral(Node** sret, Location* loc, bool value)
    {
        *sret = buildBoolLiteral(*loc, value);
    }
    void ctApi_Sparrow_mkLambdaExp(Node** sret, Location* loc, Node* parameters, Node* returnType, Node* body, Node* bodyExp, Node* closureParams)
    {
        *sret = mkLambdaExp(*loc, parameters, returnType, body, bodyExp, closureParams);
    }

    void ctApi_Sparrow_mkExpressionStmt(Node** sret, Location* loc, Node* exp)
    {
        *sret = buildExpressionStmt(*loc, exp);
    }
    void ctApi_Sparrow_mkBlockStmt(Node** sret, Location* loc, Node* statements)
    {
        *sret = buildBlockStmt(*loc, statements);
    }
    void ctApi_Sparrow_mkForStmt(Node** sret, Location* loc, StringRef name, Node* type, Node* range, Node* action)
    {
        *sret = mkForStmt(*loc, name, type, range, action);
    }
    void ctApi_Sparrow_mkReturnStmt(Node** sret, Location* loc, Node* exp)
    {
        *sret = mkReturnStmt(*loc, exp);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Compiler
    //
    bool ctApi_Compiler_registerFrontendFun(StringRef ext, StringRef funName)
    {
        int kind = SprFe_registerUserDefinedSourceCode(ext.begin, funName.begin);
        return kind >= 0;
    }
    void ctApi_Compiler_parseSprExpression(Node** sret, Location* loc, StringRef exp)
    {
        *sret = SprFe_parseSparrowExpression(*loc, exp.begin);
    }
}

void SprFrontend::registerCtApiFunctions(Backend* backend)
{
    backend->ctApiRegisterFun(backend, "$Meta.Sparrow.mkModifiers",           (void*) &ctApi_Sparrow_mkModifiers);

    backend->ctApiRegisterFun(backend, "$Meta.Sparrow.mkSprCompilationUnit",  (void*) &ctApi_Sparrow_mkSprCompilationUnit);
    backend->ctApiRegisterFun(backend, "$Meta.Sparrow.mkSprUsing",            (void*) &ctApi_Sparrow_mkSprUsing);
    backend->ctApiRegisterFun(backend, "$Meta.Sparrow.mkSprPackage",          (void*) &ctApi_Sparrow_mkSprPackage);
    backend->ctApiRegisterFun(backend, "$Meta.Sparrow.mkSprVariable",         (void*) &ctApi_Sparrow_mkSprVariable);
    backend->ctApiRegisterFun(backend, "$Meta.Sparrow.mkSprClass",            (void*) &ctApi_Sparrow_mkSprClass);

    backend->ctApiRegisterFun(backend, "$Meta.Sparrow.mkSprConcept",          (void*) &ctApi_Sparrow_mkSprConcept);

    backend->ctApiRegisterFun(backend, "$Meta.Sparrow.mkSprFunction",         (void*) &ctApi_Sparrow_mkSprFunction);
    backend->ctApiRegisterFun(backend, "$Meta.Sparrow.mkSprFunctionExp",      (void*) &ctApi_Sparrow_mkSprFunctionExp);
    backend->ctApiRegisterFun(backend, "$Meta.Sparrow.mkSprParameter",        (void*) &ctApi_Sparrow_mkSprParameter);
    backend->ctApiRegisterFun(backend, "$Meta.Sparrow.mkSprAutoParameter",    (void*) &ctApi_Sparrow_mkSprAutoParameter);

    backend->ctApiRegisterFun(backend, "$Meta.Sparrow.mkIdentifier",          (void*) &ctApi_Sparrow_mkIdentifier);
    backend->ctApiRegisterFun(backend, "$Meta.Sparrow.mkCompoundExp",         (void*) &ctApi_Sparrow_mkCompoundExp);
    backend->ctApiRegisterFun(backend, "$Meta.Sparrow.mkStarExp",             (void*) &ctApi_Sparrow_mkStarExp);
    backend->ctApiRegisterFun(backend, "$Meta.Sparrow.mkPostfixOp",           (void*) &ctApi_Sparrow_mkPostfixOp);
    backend->ctApiRegisterFun(backend, "$Meta.Sparrow.mkInfixOp",             (void*) &ctApi_Sparrow_mkInfixOp);
    backend->ctApiRegisterFun(backend, "$Meta.Sparrow.mkPrefixOp",            (void*) &ctApi_Sparrow_mkPrefixOp);
    backend->ctApiRegisterFun(backend, "$Meta.Sparrow.mkFunApplication",      (void*) &ctApi_Sparrow_mkFunApplication);
    backend->ctApiRegisterFun(backend, "$Meta.Sparrow.mkOperatorCall",        (void*) &ctApi_Sparrow_mkOperatorCall);

    backend->ctApiRegisterFun(backend, "$Meta.Sparrow.mkConditionalExp",      (void*) &ctApi_Sparrow_mkConditionalExp);
    backend->ctApiRegisterFun(backend, "$Meta.Sparrow.mkThisExp",             (void*) &ctApi_Sparrow_mkThisExp);
    backend->ctApiRegisterFun(backend, "$Meta.Sparrow.mkParenthesisExp",      (void*) &ctApi_Sparrow_mkParenthesisExp);
    backend->ctApiRegisterFun(backend, "$Meta.Sparrow.mkIntLiteral",          (void*) &ctApi_Sparrow_mkIntLiteral);
    backend->ctApiRegisterFun(backend, "$Meta.Sparrow.mkUIntLiteral",         (void*) &ctApi_Sparrow_mkUIntLiteral);
    backend->ctApiRegisterFun(backend, "$Meta.Sparrow.mkLongLiteral",         (void*) &ctApi_Sparrow_mkLongLiteral);
    backend->ctApiRegisterFun(backend, "$Meta.Sparrow.mkULongLiteral",        (void*) &ctApi_Sparrow_mkULongLiteral);
    backend->ctApiRegisterFun(backend, "$Meta.Sparrow.mkFloatLiteral",        (void*) &ctApi_Sparrow_mkFloatLiteral);
    backend->ctApiRegisterFun(backend, "$Meta.Sparrow.mkDoubleLiteral",       (void*) &ctApi_Sparrow_mkDoubleLiteral);
    backend->ctApiRegisterFun(backend, "$Meta.Sparrow.mkCharLiteral",         (void*) &ctApi_Sparrow_mkCharLiteral);
    backend->ctApiRegisterFun(backend, "$Meta.Sparrow.mkStringLiteral",       (void*) &ctApi_Sparrow_mkStringLiteral);
    backend->ctApiRegisterFun(backend, "$Meta.Sparrow.mkNullLiteral",         (void*) &ctApi_Sparrow_mkNullLiteral);
    backend->ctApiRegisterFun(backend, "$Meta.Sparrow.mkBoolLiteral",         (void*) &ctApi_Sparrow_mkBoolLiteral);
    backend->ctApiRegisterFun(backend, "$Meta.Sparrow.mkLambdaExp",           (void*) &ctApi_Sparrow_mkLambdaExp);

    backend->ctApiRegisterFun(backend, "$Meta.Sparrow.mkExpressionStmt",      (void*) &ctApi_Sparrow_mkExpressionStmt);
    backend->ctApiRegisterFun(backend, "$Meta.Sparrow.mkBlockStmt",           (void*) &ctApi_Sparrow_mkBlockStmt);
    backend->ctApiRegisterFun(backend, "$Meta.Sparrow.mkForStmt",             (void*) &ctApi_Sparrow_mkForStmt);
    backend->ctApiRegisterFun(backend, "$Meta.Sparrow.mkReturnStmt",          (void*) &ctApi_Sparrow_mkReturnStmt);

    backend->ctApiRegisterFun(backend, "$Meta.Compiler.registerFrontendFun",  (void*) &ctApi_Compiler_registerFrontendFun);
    backend->ctApiRegisterFun(backend, "$Meta.Compiler.parseSprExpression",   (void*) &ctApi_Compiler_parseSprExpression);
}

