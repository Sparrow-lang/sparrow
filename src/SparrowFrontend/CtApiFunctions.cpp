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

template <typename T> void ctApiReg(Nest_Backend* backend, const char* name, T ftor) {
    backend->ctApiRegisterFun(
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
            backend, name, (void*)ftor);
}

} // namespace

void SprFrontend::registerCtApiFunctions(Nest_Backend* backend) {
    ctApiReg(backend, "$meta.Sparrow.mkModifiers", &ctApi_Sparrow_mkModifiers);

    ctApiReg(backend, "$meta.Sparrow.mkModule", &ctApi_Sparrow_mkModule);
    ctApiReg(backend, "$meta.Sparrow.mkImportName", &ctApi_Sparrow_mkImportName);
    ctApiReg(backend, "$meta.Sparrow.mkSprUsing", &ctApi_Sparrow_mkSprUsing);
    ctApiReg(backend, "$meta.Sparrow.mkSprPackage", &ctApi_Sparrow_mkSprPackage);
    ctApiReg(backend, "$meta.Sparrow.mkSprVariable", &ctApi_Sparrow_mkSprVariable);
    ctApiReg(backend, "$meta.Sparrow.mkSprDatatype", &ctApi_Sparrow_mkSprDatatype);
    ctApiReg(backend, "$meta.Sparrow.mkSprField", &ctApi_Sparrow_mkSprField);

    ctApiReg(backend, "$meta.Sparrow.mkSprConcept", &ctApi_Sparrow_mkSprConcept);

    ctApiReg(backend, "$meta.Sparrow.mkSprFunction", &ctApi_Sparrow_mkSprFunction);
    ctApiReg(backend, "$meta.Sparrow.mkSprFunctionExp", &ctApi_Sparrow_mkSprFunctionExp);
    ctApiReg(backend, "$meta.Sparrow.mkSprParameter", &ctApi_Sparrow_mkSprParameter);
    ctApiReg(backend, "$meta.Sparrow.mkSprAutoParameter", &ctApi_Sparrow_mkSprAutoParameter);

    ctApiReg(backend, "$meta.Sparrow.mkIdentifier", &ctApi_Sparrow_mkIdentifier);
    ctApiReg(backend, "$meta.Sparrow.mkCompoundExp", &ctApi_Sparrow_mkCompoundExp);
    ctApiReg(backend, "$meta.Sparrow.mkStarExp", &ctApi_Sparrow_mkStarExp);
    ctApiReg(backend, "$meta.Sparrow.mkPostfixOp", &ctApi_Sparrow_mkPostfixOp);
    ctApiReg(backend, "$meta.Sparrow.mkInfixOp", &ctApi_Sparrow_mkInfixOp);
    ctApiReg(backend, "$meta.Sparrow.mkPrefixOp", &ctApi_Sparrow_mkPrefixOp);
    ctApiReg(backend, "$meta.Sparrow.mkFunApplication", &ctApi_Sparrow_mkFunApplication);
    ctApiReg(backend, "$meta.Sparrow.mkOperatorCall", &ctApi_Sparrow_mkOperatorCall);

    ctApiReg(backend, "$meta.Sparrow.mkConditionalExp", &ctApi_Sparrow_mkConditionalExp);
    ctApiReg(backend, "$meta.Sparrow.mkParenthesisExp", &ctApi_Sparrow_mkParenthesisExp);
    ctApiReg(backend, "$meta.Sparrow.mkIntLiteral", &ctApi_Sparrow_mkIntLiteral);
    ctApiReg(backend, "$meta.Sparrow.mkUIntLiteral", &ctApi_Sparrow_mkUIntLiteral);
    ctApiReg(backend, "$meta.Sparrow.mkLongLiteral", &ctApi_Sparrow_mkLongLiteral);
    ctApiReg(backend, "$meta.Sparrow.mkULongLiteral", &ctApi_Sparrow_mkULongLiteral);
    ctApiReg(backend, "$meta.Sparrow.mkFloatLiteral", &ctApi_Sparrow_mkFloatLiteral);
    ctApiReg(backend, "$meta.Sparrow.mkDoubleLiteral", &ctApi_Sparrow_mkDoubleLiteral);
    ctApiReg(backend, "$meta.Sparrow.mkCharLiteral", &ctApi_Sparrow_mkCharLiteral);
    ctApiReg(backend, "$meta.Sparrow.mkStringLiteral", &ctApi_Sparrow_mkStringLiteral);
    ctApiReg(backend, "$meta.Sparrow.mkNullLiteral", &ctApi_Sparrow_mkNullLiteral);
    ctApiReg(backend, "$meta.Sparrow.mkBoolLiteral", &ctApi_Sparrow_mkBoolLiteral);
    ctApiReg(backend, "$meta.Sparrow.mkLambdaExp", &ctApi_Sparrow_mkLambdaExp);

    ctApiReg(backend, "$meta.Sparrow.mkExpressionStmt", &ctApi_Sparrow_mkExpressionStmt);
    ctApiReg(backend, "$meta.Sparrow.mkBlockStmt", &ctApi_Sparrow_mkBlockStmt);
    ctApiReg(backend, "$meta.Sparrow.mkForStmt", &ctApi_Sparrow_mkForStmt);
    ctApiReg(backend, "$meta.Sparrow.mkReturnStmt", &ctApi_Sparrow_mkReturnStmt);

    ctApiReg(backend, "$meta.Compiler.registerFrontendFun", &ctApi_Compiler_registerFrontendFun);
    ctApiReg(backend, "$meta.Compiler.parseSprExpression", &ctApi_Compiler_parseSprExpression);
}
