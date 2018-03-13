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

namespace {
Node* ctApi_Sparrow_mkModifiers(Location* loc, Node* main, Node* mods) {
    return mkModifiers(*loc, main, mods);
}

Node* ctApi_Sparrow_mkModule(Location* loc, Node* moduleName, Node* declarations) {
    return mkModule(*loc, moduleName, declarations);
}
Node* ctApi_Sparrow_mkImportName(
        Location* loc, Node* moduleName, Node* importedDeclNames, StringRef alias) {
    return mkImportName(*loc, moduleName, importedDeclNames, true, alias);
}
Node* ctApi_Sparrow_mkSprUsing(Location* loc, StringRef alias, Node* usingNode) {
    return mkSprUsing(*loc, alias, usingNode);
}
Node* ctApi_Sparrow_mkSprPackage(Location* loc, StringRef name, Node* children) {
    return mkSprPackage(*loc, name, children);
}
Node* ctApi_Sparrow_mkSprVariable(Location* loc, StringRef name, Node* typeNode, Node* init) {
    return mkSprVariable(*loc, name, typeNode, init);
}
Node* ctApi_Sparrow_mkSprDatatype(Location* loc, StringRef name, Node* parameters,
        Node* baseClasses, Node* ifClause, Node* children) {
    return mkSprDatatype(*loc, name, parameters, baseClasses, ifClause, children);
}
Node* ctApi_Sparrow_mkSprField(Location* loc, StringRef name, Node* typeNode, Node* init) {
    return mkSprField(*loc, name, typeNode, init);
}

Node* ctApi_Sparrow_mkSprConcept(
        Location* loc, StringRef name, Node* baseConcept, StringRef paramName, Node* ifClause) {
    return mkSprConcept(*loc, name, paramName, baseConcept, ifClause);
}

Node* ctApi_Sparrow_mkSprFunction(Location* loc, StringRef name, Node* parameters, Node* returnType,
        Node* body, Node* ifClause) {
    return mkSprFunction(*loc, name, parameters, returnType, body, nullptr);
}
Node* ctApi_Sparrow_mkSprFunctionExp(Location* loc, StringRef name, Node* parameters,
        Node* returnType, Node* bodyExp, Node* ifClause) {
    return buildSprFunctionExp(*loc, name, parameters, returnType, bodyExp, nullptr);
}
Node* ctApi_Sparrow_mkSprParameter(Location* loc, StringRef name, Node* typeNode, Node* init) {
    return mkSprParameter(*loc, name, typeNode);
}
Node* ctApi_Sparrow_mkSprAutoParameter(Location* loc, StringRef name) {
    return mkSprAutoParameter(*loc, name);
}

Node* ctApi_Sparrow_mkIdentifier(Location* loc, StringRef id) { return mkIdentifier(*loc, id); }
Node* ctApi_Sparrow_mkCompoundExp(Location* loc, Node* base, StringRef id) {
    return mkCompoundExp(*loc, base, id);
}
Node* ctApi_Sparrow_mkStarExp(Location* loc, Node* base, StringRef operName) {
    return mkStarExp(*loc, base, operName);
}
Node* ctApi_Sparrow_mkPostfixOp(Location* loc, StringRef op, Node* base) {
    return buildPostfixOp(*loc, op, base);
}
Node* ctApi_Sparrow_mkInfixOp(Location* loc, StringRef op, Node* arg1, Node* arg2) {
    return mkInfixOp(*loc, op, arg1, arg2);
}
Node* ctApi_Sparrow_mkPrefixOp(Location* loc, StringRef op, Node* base) {
    return buildPrefixOp(*loc, op, base);
}
Node* ctApi_Sparrow_mkFunApplication(Location* loc, Node* base, Node* arguments) {
    return mkFunApplication(*loc, base, arguments);
}
Node* ctApi_Sparrow_mkOperatorCall(Location* loc, Node* arg1, StringRef op, Node* arg2) {
    return mkOperatorCall(*loc, arg1, op, arg2);
}

Node* ctApi_Sparrow_mkConditionalExp(Location* loc, Node* cond, Node* alt1, Node* alt2) {
    return mkConditionalExp(*loc, cond, alt1, alt2);
}
Node* ctApi_Sparrow_mkParenthesisExp(Location* loc, Node* exp) {
    return buildParenthesisExp(*loc, exp);
}
Node* ctApi_Sparrow_mkIntLiteral(Location* loc, int value) { return buildIntLiteral(*loc, value); }
Node* ctApi_Sparrow_mkUIntLiteral(Location* loc, unsigned int value) {
    return buildUIntLiteral(*loc, value);
}
Node* ctApi_Sparrow_mkLongLiteral(Location* loc, long value) {
    return buildLongLiteral(*loc, value);
}
Node* ctApi_Sparrow_mkULongLiteral(Location* loc, unsigned long value) {
    return buildULongLiteral(*loc, value);
}
Node* ctApi_Sparrow_mkFloatLiteral(Location* loc, float value) {
    return buildFloatLiteral(*loc, value);
}
Node* ctApi_Sparrow_mkDoubleLiteral(Location* loc, double value) {
    return buildDoubleLiteral(*loc, value);
}
Node* ctApi_Sparrow_mkCharLiteral(Location* loc, char value) {
    return buildCharLiteral(*loc, value);
}
Node* ctApi_Sparrow_mkStringLiteral(Location* loc, StringRef value) {
    return buildStringLiteral(*loc, value);
}
Node* ctApi_Sparrow_mkNullLiteral(Location* loc) { return buildNullLiteral(*loc); }
Node* ctApi_Sparrow_mkBoolLiteral(Location* loc, bool value) {
    return buildBoolLiteral(*loc, value);
}
Node* ctApi_Sparrow_mkLambdaExp(Location* loc, Node* parameters, Node* returnType, Node* body,
        Node* bodyExp, Node* closureParams) {
    return mkLambdaExp(*loc, parameters, returnType, body, bodyExp, closureParams);
}

Node* ctApi_Sparrow_mkExpressionStmt(Location* loc, Node* exp) {
    return buildExpressionStmt(*loc, exp);
}
Node* ctApi_Sparrow_mkBlockStmt(Location* loc, Node* statements) {
    return buildBlockStmt(*loc, statements);
}
Node* ctApi_Sparrow_mkForStmt(
        Location* loc, StringRef name, Node* type, Node* range, Node* action) {
    return mkForStmt(*loc, name, type, range, action);
}
Node* ctApi_Sparrow_mkReturnStmt(Location* loc, Node* exp) { return mkReturnStmt(*loc, exp); }

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Compiler
//
bool ctApi_Compiler_registerFrontendFun(StringRef ext, StringRef funName) {
    int kind = SprFe_registerUserDefinedSourceCode(ext.begin, funName.begin);
    return kind >= 0;
}
Node* ctApi_Compiler_parseSprExpression(Location* loc, StringRef exp) {
    return SprFe_parseSparrowExpression(*loc, exp.begin);
}
} // namespace

void SprFrontend::registerCtApiFunctions(Backend* backend) {
    backend->ctApiRegisterFun(
            backend, "$meta.Sparrow.mkModifiers", (void*)&ctApi_Sparrow_mkModifiers);

    backend->ctApiRegisterFun(backend, "$meta.Sparrow.mkModule", (void*)&ctApi_Sparrow_mkModule);
    backend->ctApiRegisterFun(
            backend, "$meta.Sparrow.mkImportName", (void*)&ctApi_Sparrow_mkImportName);
    backend->ctApiRegisterFun(
            backend, "$meta.Sparrow.mkSprUsing", (void*)&ctApi_Sparrow_mkSprUsing);
    backend->ctApiRegisterFun(
            backend, "$meta.Sparrow.mkSprPackage", (void*)&ctApi_Sparrow_mkSprPackage);
    backend->ctApiRegisterFun(
            backend, "$meta.Sparrow.mkSprVariable", (void*)&ctApi_Sparrow_mkSprVariable);
    backend->ctApiRegisterFun(
            backend, "$meta.Sparrow.mkSprDatatype", (void*)&ctApi_Sparrow_mkSprDatatype);
    backend->ctApiRegisterFun(
            backend, "$meta.Sparrow.mkSprField", (void*)&ctApi_Sparrow_mkSprField);

    backend->ctApiRegisterFun(
            backend, "$meta.Sparrow.mkSprConcept", (void*)&ctApi_Sparrow_mkSprConcept);

    backend->ctApiRegisterFun(
            backend, "$meta.Sparrow.mkSprFunction", (void*)&ctApi_Sparrow_mkSprFunction);
    backend->ctApiRegisterFun(
            backend, "$meta.Sparrow.mkSprFunctionExp", (void*)&ctApi_Sparrow_mkSprFunctionExp);
    backend->ctApiRegisterFun(
            backend, "$meta.Sparrow.mkSprParameter", (void*)&ctApi_Sparrow_mkSprParameter);
    backend->ctApiRegisterFun(
            backend, "$meta.Sparrow.mkSprAutoParameter", (void*)&ctApi_Sparrow_mkSprAutoParameter);

    backend->ctApiRegisterFun(
            backend, "$meta.Sparrow.mkIdentifier", (void*)&ctApi_Sparrow_mkIdentifier);
    backend->ctApiRegisterFun(
            backend, "$meta.Sparrow.mkCompoundExp", (void*)&ctApi_Sparrow_mkCompoundExp);
    backend->ctApiRegisterFun(backend, "$meta.Sparrow.mkStarExp", (void*)&ctApi_Sparrow_mkStarExp);
    backend->ctApiRegisterFun(
            backend, "$meta.Sparrow.mkPostfixOp", (void*)&ctApi_Sparrow_mkPostfixOp);
    backend->ctApiRegisterFun(backend, "$meta.Sparrow.mkInfixOp", (void*)&ctApi_Sparrow_mkInfixOp);
    backend->ctApiRegisterFun(
            backend, "$meta.Sparrow.mkPrefixOp", (void*)&ctApi_Sparrow_mkPrefixOp);
    backend->ctApiRegisterFun(
            backend, "$meta.Sparrow.mkFunApplication", (void*)&ctApi_Sparrow_mkFunApplication);
    backend->ctApiRegisterFun(
            backend, "$meta.Sparrow.mkOperatorCall", (void*)&ctApi_Sparrow_mkOperatorCall);

    backend->ctApiRegisterFun(
            backend, "$meta.Sparrow.mkConditionalExp", (void*)&ctApi_Sparrow_mkConditionalExp);
    backend->ctApiRegisterFun(
            backend, "$meta.Sparrow.mkParenthesisExp", (void*)&ctApi_Sparrow_mkParenthesisExp);
    backend->ctApiRegisterFun(
            backend, "$meta.Sparrow.mkIntLiteral", (void*)&ctApi_Sparrow_mkIntLiteral);
    backend->ctApiRegisterFun(
            backend, "$meta.Sparrow.mkUIntLiteral", (void*)&ctApi_Sparrow_mkUIntLiteral);
    backend->ctApiRegisterFun(
            backend, "$meta.Sparrow.mkLongLiteral", (void*)&ctApi_Sparrow_mkLongLiteral);
    backend->ctApiRegisterFun(
            backend, "$meta.Sparrow.mkULongLiteral", (void*)&ctApi_Sparrow_mkULongLiteral);
    backend->ctApiRegisterFun(
            backend, "$meta.Sparrow.mkFloatLiteral", (void*)&ctApi_Sparrow_mkFloatLiteral);
    backend->ctApiRegisterFun(
            backend, "$meta.Sparrow.mkDoubleLiteral", (void*)&ctApi_Sparrow_mkDoubleLiteral);
    backend->ctApiRegisterFun(
            backend, "$meta.Sparrow.mkCharLiteral", (void*)&ctApi_Sparrow_mkCharLiteral);
    backend->ctApiRegisterFun(
            backend, "$meta.Sparrow.mkStringLiteral", (void*)&ctApi_Sparrow_mkStringLiteral);
    backend->ctApiRegisterFun(
            backend, "$meta.Sparrow.mkNullLiteral", (void*)&ctApi_Sparrow_mkNullLiteral);
    backend->ctApiRegisterFun(
            backend, "$meta.Sparrow.mkBoolLiteral", (void*)&ctApi_Sparrow_mkBoolLiteral);
    backend->ctApiRegisterFun(
            backend, "$meta.Sparrow.mkLambdaExp", (void*)&ctApi_Sparrow_mkLambdaExp);

    backend->ctApiRegisterFun(
            backend, "$meta.Sparrow.mkExpressionStmt", (void*)&ctApi_Sparrow_mkExpressionStmt);
    backend->ctApiRegisterFun(
            backend, "$meta.Sparrow.mkBlockStmt", (void*)&ctApi_Sparrow_mkBlockStmt);
    backend->ctApiRegisterFun(backend, "$meta.Sparrow.mkForStmt", (void*)&ctApi_Sparrow_mkForStmt);
    backend->ctApiRegisterFun(
            backend, "$meta.Sparrow.mkReturnStmt", (void*)&ctApi_Sparrow_mkReturnStmt);

    backend->ctApiRegisterFun(backend, "$meta.Compiler.registerFrontendFun",
            (void*)&ctApi_Compiler_registerFrontendFun);
    backend->ctApiRegisterFun(backend, "$meta.Compiler.parseSprExpression",
            (void*)&ctApi_Compiler_parseSprExpression);
}
