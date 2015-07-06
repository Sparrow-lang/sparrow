#include <StdInc.h>
#include "CtApiFunctions.h"
#include "UserDefinedSourceCode.h"
#include "SparrowSourceCode.h"

#include <Nodes/SparrowNodes.h>
#include <Feather/Util/StringData.h>

#include <Nest/Backend/Backend.h>

using namespace SprFrontend;
using namespace Feather;
using namespace Nest;

namespace
{
    void ctApi_Sparrow_mkModifiers(DynNode** sret, Location* loc, DynNode* main, DynNode* mods)
    {
        *sret = mkModifiers(*loc, main, mods);
    }

    void ctApi_Sparrow_mkSprCompilationUnit(DynNode** sret, Location* loc, DynNode* package, NodeList* imports, NodeList* declarations)
    {
        *sret = mkSprCompilationUnit(*loc, package, imports, declarations);
    }
    void ctApi_Sparrow_mkSprUsing(DynNode** sret, Location* loc, StringData alias, DynNode* usingNode)
    {
        *sret = mkSprUsing(*loc, alias.toStdString(), usingNode);
    }
    void ctApi_Sparrow_mkSprPackage(DynNode** sret, Location* loc, StringData name, NodeList* children)
    {
        *sret = mkSprPackage(*loc, name.toStdString(), children);
    }
    void ctApi_Sparrow_mkSprVariable(DynNode** sret, Location* loc, StringData name, DynNode* typeNode, DynNode* init)
    {
        *sret = mkSprVariable(*loc, name.toStdString(), typeNode, init);
    }
    void ctApi_Sparrow_mkSprClass(DynNode** sret, Location* loc, StringData name, NodeList* parameters, NodeList* baseClasses, DynNode* ifClause, NodeList* children)
    {
        *sret = mkSprClass(*loc, name.toStdString(), parameters, baseClasses, ifClause, children);
    }

    void ctApi_Sparrow_mkSprConcept(DynNode** sret, Location* loc, StringData name, DynNode* baseConcept, StringData paramName, DynNode* ifClause)
    {
        *sret = mkSprConcept(*loc, name.toStdString(), paramName.toStdString(), baseConcept, ifClause);
    }

    void ctApi_Sparrow_mkSprFunction(DynNode** sret, Location* loc, StringData name, NodeList* parameters, DynNode* returnType, DynNode* body, DynNode* ifClause)
    {
        *sret = mkSprFunction(*loc, name.toStdString(), parameters, returnType, body);
    }
    void ctApi_Sparrow_mkSprFunctionExp(DynNode** sret, Location* loc, StringData name, NodeList* parameters, DynNode* returnType, DynNode* bodyExp, DynNode* ifClause)
    {
        *sret = mkSprFunctionExp(*loc, name.toStdString(), parameters, returnType, bodyExp);
    }
    void ctApi_Sparrow_mkSprParameter(DynNode** sret, Location* loc, StringData name, DynNode* typeNode, DynNode* init)
    {
        *sret = mkSprParameter(*loc, name.toStdString(), typeNode);
    }
    void ctApi_Sparrow_mkSprAutoParameter(DynNode** sret, Location* loc, StringData name)
    {
        *sret = mkSprAutoParameter(*loc, name.toStdString());
    }

    void ctApi_Sparrow_mkIdentifier(DynNode** sret, Location* loc, StringData id)
    {
        *sret = mkIdentifier(*loc, id.toStdString());
    }
    void ctApi_Sparrow_mkCompoundExp(DynNode** sret, Location* loc, DynNode* base, StringData id)
    {
        *sret = mkCompoundExp(*loc, base, id.toStdString());
    }
    void ctApi_Sparrow_mkStarExp(DynNode** sret, Location* loc, DynNode* base, StringData operName)
    {
        *sret = mkStarExp(*loc, base, operName.toStdString());
    }
    void ctApi_Sparrow_mkPostfixOp(DynNode** sret, Location* loc, StringData op, DynNode* base)
    {
        *sret = mkPostfixOp(*loc, op.toStdString(), base);
    }
    void ctApi_Sparrow_mkInfixOp(DynNode** sret, Location* loc, StringData op, DynNode* arg1, DynNode* arg2)
    {
        *sret = mkInfixOp(*loc, op.toStdString(), arg1, arg2);
    }
    void ctApi_Sparrow_mkPrefixOp(DynNode** sret, Location* loc, StringData op, DynNode* base)
    {
        *sret = mkPrefixOp(*loc, op.toStdString(), base);
    }
    void ctApi_Sparrow_mkFunApplication(DynNode** sret, Location* loc, DynNode* base, NodeList* arguments)
    {
        *sret = mkFunApplication(*loc, base, arguments);
    }
    void ctApi_Sparrow_mkOperatorCall(DynNode** sret, Location* loc, DynNode* arg1, StringData op, DynNode* arg2)
    {
        *sret = mkOperatorCall(*loc, arg1, op.toStdString(), arg2);
    }

    void ctApi_Sparrow_mkConditionalExp(DynNode** sret, Location* loc, DynNode* cond, DynNode* alt1, DynNode* alt2)
    {
        *sret = mkConditionalExp(*loc, cond, alt1, alt2);
    }
    void ctApi_Sparrow_mkThisExp(DynNode** sret, Location* loc)
    {
        *sret = mkThisExp(*loc);
    }
    void ctApi_Sparrow_mkParenthesisExp(DynNode** sret, Location* loc, DynNode* exp)
    {
        *sret = mkParenthesisExp(*loc, exp);
    }
    void ctApi_Sparrow_mkIntLiteral(DynNode** sret, Location* loc, int value)
    {
        *sret = mkIntLiteral(*loc, value);
    }
    void ctApi_Sparrow_mkUIntLiteral(DynNode** sret, Location* loc, unsigned int value)
    {
        *sret = mkUIntLiteral(*loc, value);
    }
    void ctApi_Sparrow_mkLongLiteral(DynNode** sret, Location* loc, long value)
    {
        *sret = mkLongLiteral(*loc, value);
    }
    void ctApi_Sparrow_mkULongLiteral(DynNode** sret, Location* loc, unsigned long value)
    {
        *sret = mkULongLiteral(*loc, value);
    }
    void ctApi_Sparrow_mkFloatLiteral(DynNode** sret, Location* loc, float value)
    {
        *sret = mkFloatLiteral(*loc, value);
    }
    void ctApi_Sparrow_mkDoubleLiteral(DynNode** sret, Location* loc, double value)
    {
        *sret = mkDoubleLiteral(*loc, value);
    }
    void ctApi_Sparrow_mkCharLiteral(DynNode** sret, Location* loc, char value)
    {
        *sret = mkCharLiteral(*loc, value);
    }
    void ctApi_Sparrow_mkStringLiteral(DynNode** sret, Location* loc, StringData value)
    {
        *sret = mkStringLiteral(*loc, value.toStdString());
    }
    void ctApi_Sparrow_mkNullLiteral(DynNode** sret, Location* loc)
    {
        *sret = mkNullLiteral(*loc);
    }
    void ctApi_Sparrow_mkBoolLiteral(DynNode** sret, Location* loc, bool value)
    {
        *sret = mkBoolLiteral(*loc, value);
    }
    void ctApi_Sparrow_mkLambdaExp(DynNode** sret, Location* loc, NodeList* parameters, DynNode* returnType, DynNode* body, DynNode* bodyExp, NodeList* closureParams)
    {
        *sret = mkLambdaExp(*loc, parameters, returnType, body, bodyExp, closureParams);
    }

    void ctApi_Sparrow_mkExpressionStmt(DynNode** sret, Location* loc, DynNode* exp)
    {
        *sret = mkExpressionStmt(*loc, exp);
    }
    void ctApi_Sparrow_mkBlockStmt(DynNode** sret, Location* loc, NodeList* statements)
    {
        *sret = mkBlockStmt(*loc, statements);
    }
    void ctApi_Sparrow_mkForStmt(DynNode** sret, Location* loc, StringData name, DynNode* type, DynNode* range, DynNode* action)
    {
        *sret = mkForStmt(*loc, name.toStdString(), type, range, action);
    }
    void ctApi_Sparrow_mkReturnStmt(DynNode** sret, Location* loc, DynNode* exp)
    {
        *sret = mkReturnStmt(*loc, exp);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Compiler
    //
    bool ctApi_Compiler_registerFrontendFun(StringData ext, StringData funName)
    {
        UserDefinedSourceCode::registerSelf(ext.toStdString(), funName.toStdString());
        return true;
    }
    void ctApi_Compiler_parseSprExpression(DynNode** sret, Location* loc, StringData exp)
    {
        const SparrowSourceCode* ssc = dynamic_cast<const SparrowSourceCode*>(loc->sourceCode());
        if ( ssc )
            *sret = ssc->parseExpression(*loc, exp.toStdString());
        else
            *sret = nullptr;
    }
}

void SprFrontend::registerCtApiFunctions(Backend& backend)
{
    backend.ctApiRegisterFun("$Meta.Sparrow.mkModifiers",           (void*) &ctApi_Sparrow_mkModifiers);

    backend.ctApiRegisterFun("$Meta.Sparrow.mkSprCompilationUnit",  (void*) &ctApi_Sparrow_mkSprCompilationUnit);
    backend.ctApiRegisterFun("$Meta.Sparrow.mkSprUsing",            (void*) &ctApi_Sparrow_mkSprUsing);
    backend.ctApiRegisterFun("$Meta.Sparrow.mkSprPackage",          (void*) &ctApi_Sparrow_mkSprPackage);
    backend.ctApiRegisterFun("$Meta.Sparrow.mkSprVariable",         (void*) &ctApi_Sparrow_mkSprVariable);
    backend.ctApiRegisterFun("$Meta.Sparrow.mkSprClass",            (void*) &ctApi_Sparrow_mkSprClass);

    backend.ctApiRegisterFun("$Meta.Sparrow.mkSprConcept",          (void*) &ctApi_Sparrow_mkSprConcept);

    backend.ctApiRegisterFun("$Meta.Sparrow.mkSprFunction",         (void*) &ctApi_Sparrow_mkSprFunction);
    backend.ctApiRegisterFun("$Meta.Sparrow.mkSprFunctionExp",      (void*) &ctApi_Sparrow_mkSprFunctionExp);
    backend.ctApiRegisterFun("$Meta.Sparrow.mkSprParameter",        (void*) &ctApi_Sparrow_mkSprParameter);
    backend.ctApiRegisterFun("$Meta.Sparrow.mkSprAutoParameter",    (void*) &ctApi_Sparrow_mkSprAutoParameter);

    backend.ctApiRegisterFun("$Meta.Sparrow.mkIdentifier",          (void*) &ctApi_Sparrow_mkIdentifier);
    backend.ctApiRegisterFun("$Meta.Sparrow.mkCompoundExp",         (void*) &ctApi_Sparrow_mkCompoundExp);
    backend.ctApiRegisterFun("$Meta.Sparrow.mkStarExp",             (void*) &ctApi_Sparrow_mkStarExp);
    backend.ctApiRegisterFun("$Meta.Sparrow.mkPostfixOp",           (void*) &ctApi_Sparrow_mkPostfixOp);
    backend.ctApiRegisterFun("$Meta.Sparrow.mkInfixOp",             (void*) &ctApi_Sparrow_mkInfixOp);
    backend.ctApiRegisterFun("$Meta.Sparrow.mkPrefixOp",            (void*) &ctApi_Sparrow_mkPrefixOp);
    backend.ctApiRegisterFun("$Meta.Sparrow.mkFunApplication",      (void*) &ctApi_Sparrow_mkFunApplication);
    backend.ctApiRegisterFun("$Meta.Sparrow.mkOperatorCall",        (void*) &ctApi_Sparrow_mkOperatorCall);

    backend.ctApiRegisterFun("$Meta.Sparrow.mkConditionalExp",      (void*) &ctApi_Sparrow_mkConditionalExp);
    backend.ctApiRegisterFun("$Meta.Sparrow.mkThisExp",             (void*) &ctApi_Sparrow_mkThisExp);
    backend.ctApiRegisterFun("$Meta.Sparrow.mkParenthesisExp",      (void*) &ctApi_Sparrow_mkParenthesisExp);
    backend.ctApiRegisterFun("$Meta.Sparrow.mkIntLiteral",          (void*) &ctApi_Sparrow_mkIntLiteral);
    backend.ctApiRegisterFun("$Meta.Sparrow.mkUIntLiteral",         (void*) &ctApi_Sparrow_mkUIntLiteral);
    backend.ctApiRegisterFun("$Meta.Sparrow.mkLongLiteral",         (void*) &ctApi_Sparrow_mkLongLiteral);
    backend.ctApiRegisterFun("$Meta.Sparrow.mkULongLiteral",        (void*) &ctApi_Sparrow_mkULongLiteral);
    backend.ctApiRegisterFun("$Meta.Sparrow.mkFloatLiteral",        (void*) &ctApi_Sparrow_mkFloatLiteral);
    backend.ctApiRegisterFun("$Meta.Sparrow.mkDoubleLiteral",       (void*) &ctApi_Sparrow_mkDoubleLiteral);
    backend.ctApiRegisterFun("$Meta.Sparrow.mkCharLiteral",         (void*) &ctApi_Sparrow_mkCharLiteral);
    backend.ctApiRegisterFun("$Meta.Sparrow.mkStringLiteral",       (void*) &ctApi_Sparrow_mkStringLiteral);
    backend.ctApiRegisterFun("$Meta.Sparrow.mkNullLiteral",         (void*) &ctApi_Sparrow_mkNullLiteral);
    backend.ctApiRegisterFun("$Meta.Sparrow.mkBoolLiteral",         (void*) &ctApi_Sparrow_mkBoolLiteral);
    backend.ctApiRegisterFun("$Meta.Sparrow.mkLambdaExp",           (void*) &ctApi_Sparrow_mkLambdaExp);

    backend.ctApiRegisterFun("$Meta.Sparrow.mkExpressionStmt",      (void*) &ctApi_Sparrow_mkExpressionStmt);
    backend.ctApiRegisterFun("$Meta.Sparrow.mkBlockStmt",           (void*) &ctApi_Sparrow_mkBlockStmt);
    backend.ctApiRegisterFun("$Meta.Sparrow.mkForStmt",             (void*) &ctApi_Sparrow_mkForStmt);
    backend.ctApiRegisterFun("$Meta.Sparrow.mkReturnStmt",          (void*) &ctApi_Sparrow_mkReturnStmt);

    backend.ctApiRegisterFun("$Meta.Compiler.registerFrontendFun",  (void*) &ctApi_Compiler_registerFrontendFun);
    backend.ctApiRegisterFun("$Meta.Compiler.parseSprExpression",   (void*) &ctApi_Compiler_parseSprExpression);
}

