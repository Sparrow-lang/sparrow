#include "StdInc.h"
#include "Parser.h"
#include "Nodes/Builder.h"
#include "Helpers/DeclsHelpers.h"
#include "Nest/Api/SourceCode.h"
#include "SparrowFrontend/Nodes/Decl.hpp"

using namespace SprFrontend;

namespace SprFrontend {

//! Represents a parser context.
//! Datatype defined in the Sparrow code.
//! We don't care about the internal structure.
struct ParserContext {};

//! Represent a Sparrow string object.
//! Datatype defined in the Sparrow code.
//! We need to access its contents, but we don't want to import all the functions assoc with it.
struct SprString {
    const char* begin;
    const char* end;
    const char* endOfStore;
};

//! Sparrow parser interface for reporting errors
struct ErrorReporter {
    void* self;
    void (*reportErrorFn)(ErrorReporter*, const Location*, StringRef);
};

//! Sparrow parser interface for building the AST
struct AstBuilder {
    void* self;

    Node* (*addToNodeListFn)(AstBuilder*, Node*, Node*);

    Node* (*mkModifiersFn)(AstBuilder*, const Location*, Node*, Node*);
    Node* (*mkModuleFn)(AstBuilder*, const Location*, Node*, Node*);
    Node* (*mkImportNameFn)(AstBuilder*, const Location*, StringRef, Node*, Node*);
    Node* (*mkUsingFn)(AstBuilder*, const Location*, StringRef, Node*);
    Node* (*mkPackageFn)(AstBuilder*, const Location*, StringRef, Node*, Node*, Node*);
    Node* (*mkDatatypeFn)(AstBuilder*, const Location*, StringRef, Node*, Node*, Node*, Node*);
    Node* (*mkFieldFn)(AstBuilder*, const Location*, StringRef, Node*, Node*);
    Node* (*mkConceptFn)(AstBuilder*, const Location*, StringRef, StringRef, Node*, Node*);
    Node* (*mkLetFn)(AstBuilder*, const Location*, StringRef, Node*, Node*);
    Node* (*mkVarFn)(AstBuilder*, const Location*, StringRef, Node*, Node*);
    Node* (*mkParameterFn)(AstBuilder*, const Location*, StringRef, Node*, Node*);
    Node* (*mkFunFn)(AstBuilder*, const Location*, StringRef, Node*, Node*, Node*, Node*, Node*);

    Node* (*mkParenthesisExprFn)(AstBuilder*, Node*);
    Node* (*mkPostfixOpFn)(AstBuilder*, const Location*, Node*, StringRef);
    Node* (*mkInfixOpFn)(AstBuilder*, const Location*, Node*, StringRef, Node*);
    Node* (*mkPrefixOpFn)(AstBuilder*, const Location*, StringRef, Node*);
    Node* (*mkIdentifierFn)(AstBuilder*, const Location*, StringRef);
    Node* (*mkCompoundExprFn)(AstBuilder*, const Location*, Node*, StringRef);
    Node* (*mkStarExprFn)(AstBuilder*, const Location*, Node*, StringRef);
    Node* (*mkDotExprFn)(AstBuilder*, const Location*, Node*, StringRef);
    Node* (*mkFunAppExprFn)(AstBuilder*, const Location*, Node*, Node*);
    Node* (*mkLambdaExprFn)(AstBuilder*, const Location*, Node*, Node*, Node*, Node*, Node*);
    Node* (*mkNullLiteralFn)(AstBuilder*, const Location*);
    Node* (*mkBoolLiteralFn)(AstBuilder*, const Location*, bool);
    Node* (*mkIntLiteralFn)(AstBuilder*, const Location*, int);
    Node* (*mkUIntLiteralFn)(AstBuilder*, const Location*, unsigned int);
    Node* (*mkLongLiteralFn)(AstBuilder*, const Location*, long long);
    Node* (*mkULongLiteralFn)(AstBuilder*, const Location*, unsigned long long);
    Node* (*mkFloatLiteralFn)(AstBuilder*, const Location*, float);
    Node* (*mkDoubleLiteralFn)(AstBuilder*, const Location*, double);
    Node* (*mkCharLiteralFn)(AstBuilder*, const Location*, char);
    Node* (*mkStringLiteralFn)(AstBuilder*, const Location*, StringRef);

    Node* (*mkBlockStmtFn)(AstBuilder*, const Location*, Node*);
    Node* (*mkIfStmtFn)(AstBuilder*, const Location*, Node*, Node*, Node*);
    Node* (*mkForStmtFn)(AstBuilder*, const Location*, StringRef, Node*, Node*, Node*);
    Node* (*mkWhileStmtFn)(AstBuilder*, const Location*, Node*, Node*, Node*);
    Node* (*mkBreakStmtFn)(AstBuilder*, const Location*);
    Node* (*mkContinueStmtFn)(AstBuilder*, const Location*);
    Node* (*mkReturnStmtFn)(AstBuilder*, const Location*, Node*);
};

//! Compiler implementation or error reporter
struct CompilerErrorReporter : ErrorReporter {
    CompilerErrorReporter() {
        self = this;
        reportErrorFn = &CompilerErrorReporter::reportError;
    }

    static void reportError(ErrorReporter* self, const Location* loc, StringRef msg) {
        Nest_reportDiagnostic(*loc, diagError, msg.begin);
    }
};

//! Compiler implementation or error reporter
struct CompilerAstBuilder : AstBuilder {
    CompilerAstBuilder() {
        self = this;
        addToNodeListFn = &CompilerAstBuilder::addToNodeList_Impl;

        mkModifiersFn = &CompilerAstBuilder::mkModifiers_Impl;
        mkModuleFn = &CompilerAstBuilder::mkModule_Impl;
        mkImportNameFn = &CompilerAstBuilder::mkImportName_Impl;
        mkUsingFn = &CompilerAstBuilder::mkUsing_Impl;
        mkPackageFn = &CompilerAstBuilder::mkPackage_Impl;
        mkDatatypeFn = &CompilerAstBuilder::mkDatatype_Impl;
        mkFieldFn = &CompilerAstBuilder::mkField_Impl;
        mkConceptFn = &CompilerAstBuilder::mkConcept_Impl;
        mkLetFn = &CompilerAstBuilder::mkLet_Impl;
        mkVarFn = &CompilerAstBuilder::mkVar_Impl;
        mkParameterFn = &CompilerAstBuilder::mkParameter_Impl;
        mkFunFn = &CompilerAstBuilder::mkFun_Impl;

        mkParenthesisExprFn = &CompilerAstBuilder::mkParenthesisExpr_Impl;
        mkPostfixOpFn = &CompilerAstBuilder::mkPostfixOp_Impl;
        mkInfixOpFn = &CompilerAstBuilder::mkInfixOp_Impl;
        mkPrefixOpFn = &CompilerAstBuilder::mkPrefixOp_Impl;
        mkIdentifierFn = &CompilerAstBuilder::mkIdentifier_Impl;
        mkCompoundExprFn = &CompilerAstBuilder::mkCompoundExpr_Impl;
        mkStarExprFn = &CompilerAstBuilder::mkStarExpr_Impl;
        mkDotExprFn = &CompilerAstBuilder::mkDotExpr_Impl;
        mkFunAppExprFn = &CompilerAstBuilder::mkFunAppExpr_Impl;
        mkLambdaExprFn = &CompilerAstBuilder::mkLambdaExpr_Impl;
        mkNullLiteralFn = &CompilerAstBuilder::mkNullLiteral_Impl;
        mkBoolLiteralFn = &CompilerAstBuilder::mkBoolLiteral_Impl;
        mkIntLiteralFn = &CompilerAstBuilder::mkIntLiteral_Impl;
        mkUIntLiteralFn = &CompilerAstBuilder::mkUIntLiteral_Impl;
        mkLongLiteralFn = &CompilerAstBuilder::mkLongLiteral_Impl;
        mkULongLiteralFn = &CompilerAstBuilder::mkULongLiteral_Impl;
        mkFloatLiteralFn = &CompilerAstBuilder::mkFloatLiteral_Impl;
        mkDoubleLiteralFn = &CompilerAstBuilder::mkDoubleLiteral_Impl;
        mkCharLiteralFn = &CompilerAstBuilder::mkCharLiteral_Impl;
        mkStringLiteralFn = &CompilerAstBuilder::mkStringLiteral_Impl;

        mkBlockStmtFn = &CompilerAstBuilder::mkBlockStmt_Impl;
        mkIfStmtFn = &CompilerAstBuilder::mkIfStmt_Impl;
        mkForStmtFn = &CompilerAstBuilder::mkForStmt_Impl;
        mkWhileStmtFn = &CompilerAstBuilder::mkWhileStmt_Impl;
        mkBreakStmtFn = &CompilerAstBuilder::mkBreakStmt_Impl;
        mkContinueStmtFn = &CompilerAstBuilder::mkContinueStmt_Impl;
        mkReturnStmtFn = &CompilerAstBuilder::mkReturnStmt_Impl;
    }

    static Node* addToNodeList_Impl(AstBuilder*, Node* nl, Node* newNode) {
        return Feather_addToNodeList(nl, newNode);
    }

    static Node* mkModifiers_Impl(AstBuilder*, const Location* loc, Node* main, Node* mods) {
        return mkModifiers(*loc, main, mods);
    }
    static Node* mkModule_Impl(AstBuilder*, const Location* loc, Node* moduleName, Node* decls) {
        return mkModule(*loc, moduleName, decls);
    }
    static Node* mkImportName_Impl(
            AstBuilder*, const Location* loc, StringRef alias, Node* toImport, Node* decls) {
        return mkImportName(*loc, toImport, decls, true, alias);
    }
    static Node* mkUsing_Impl(AstBuilder*, const Location* loc, StringRef alias, Node* usingNode) {
        return mkSprUsing(*loc, alias, usingNode);
    }
    static Node* mkPackage_Impl(AstBuilder*, const Location* loc, StringRef name, Node* children,
            Node* params, Node* ifClause) {
        return mkSprPackage(*loc, name, children, params, ifClause);
    }
    static Node* mkDatatype_Impl(AstBuilder*, const Location* loc, StringRef name, Node* params,
            Node* underlyingData, Node* ifClause, Node* children) {
        return mkSprDatatype(*loc, name, params, underlyingData, ifClause, children);
    }
    static Node* mkField_Impl(
            AstBuilder*, const Location* loc, StringRef name, Node* typeNode, Node* init) {
        return mkSprField(*loc, name, typeNode, init);
    }
    static Node* mkConcept_Impl(AstBuilder*, const Location* loc, StringRef name,
            StringRef paramName, Node* baseConcept, Node* ifClause) {
        return mkSprConcept(*loc, name, paramName, baseConcept, ifClause);
    }
    static Node* mkLet_Impl(
            AstBuilder*, const Location* loc, StringRef name, Node* typeNode, Node* init) {
        return VariableDecl::createConst(*loc, name, typeNode, init);
    }
    static Node* mkVar_Impl(
            AstBuilder*, const Location* loc, StringRef name, Node* typeNode, Node* init) {
        return VariableDecl::createMut(*loc, name, typeNode, init);
    }
    static Node* mkParameter_Impl(
            AstBuilder*, const Location* loc, StringRef name, Node* typeNode, Node* init) {
        return mkSprParameter(*loc, name, typeNode, init);
    }
    static Node* mkFun_Impl(AstBuilder*, const Location* loc, StringRef name, Node* formals,
            Node* retType, Node* body, Node* bodyExp, Node* ifClause) {
        if (bodyExp && !body) {
            const Location& loc2 = bodyExp->location;
            body = Feather_mkLocalSpace(*loc, fromIniList({mkReturnStmt(loc2, bodyExp)}));
            if (!retType)
                retType = mkFunApplication(loc2, mkIdentifier(loc2, StringRef("typeOf")),
                        Feather_mkNodeList(loc2, fromIniList({bodyExp})));
        }
        return mkSprFunction(*loc, name, formals, retType, body, ifClause);
    }

    static Node* mkParenthesisExpr_Impl(AstBuilder*, Node* expr) {
        return Feather_mkNodeList(expr ? expr->location : NOLOC, fromIniList({expr}));
    }
    static Node* mkPostfixOp_Impl(AstBuilder*, const Location* loc, Node* base, StringRef op) {
        return mkOperatorCall(*loc, base, op, nullptr);
    }
    static Node* mkInfixOp_Impl(
            AstBuilder*, const Location* loc, Node* lhs, StringRef op, Node* rhs) {
        return mkInfixOp(*loc, op, lhs, rhs);
    }
    static Node* mkPrefixOp_Impl(AstBuilder*, const Location* loc, StringRef op, Node* base) {
        return mkInfixOp(*loc, op, nullptr, base);
    }
    static Node* mkIdentifier_Impl(AstBuilder*, const Location* loc, StringRef id) {
        return mkIdentifier(*loc, id);
    }
    static Node* mkCompoundExpr_Impl(AstBuilder*, const Location* loc, Node* base, StringRef id) {
        return mkCompoundExp(*loc, base, id);
    }
    static Node* mkStarExpr_Impl(AstBuilder*, const Location* loc, Node* base, StringRef id) {
        return mkStarExp(*loc, base, id);
    }
    static Node* mkDotExpr_Impl(AstBuilder*, const Location* loc, Node* base, StringRef id) {
        return mkCompoundExp(*loc, base, id);
    }
    static Node* mkFunAppExpr_Impl(AstBuilder*, const Location* loc, Node* base, Node* args) {
        return mkFunApplication(*loc, base, args);
    }
    static Node* mkLambdaExpr_Impl(AstBuilder*, const Location* loc, Node* closureParams,
            Node* formals, Node* retType, Node* body, Node* bodyExpr) {
        return mkLambdaExp(*loc, formals, retType, body, bodyExpr, closureParams);
    }
    static Node* mkNullLiteral_Impl(AstBuilder*, const Location* loc) {
        return buildNullLiteral(*loc);
    }
    static Node* mkBoolLiteral_Impl(AstBuilder*, const Location* loc, bool val) {
        return buildBoolLiteral(*loc, val);
    }
    static Node* mkIntLiteral_Impl(AstBuilder*, const Location* loc, int val) {
        return buildIntLiteral(*loc, val);
    }
    static Node* mkUIntLiteral_Impl(AstBuilder*, const Location* loc, unsigned int val) {
        return buildUIntLiteral(*loc, val);
    }
    static Node* mkLongLiteral_Impl(AstBuilder*, const Location* loc, long long val) {
        return buildLongLiteral(*loc, val);
    }
    static Node* mkULongLiteral_Impl(AstBuilder*, const Location* loc, unsigned long long val) {
        return buildULongLiteral(*loc, val);
    }
    static Node* mkFloatLiteral_Impl(AstBuilder*, const Location* loc, float val) {
        return buildFloatLiteral(*loc, val);
    }
    static Node* mkDoubleLiteral_Impl(AstBuilder*, const Location* loc, double val) {
        return buildDoubleLiteral(*loc, val);
    }
    static Node* mkCharLiteral_Impl(AstBuilder*, const Location* loc, char val) {
        return buildCharLiteral(*loc, val);
    }
    static Node* mkStringLiteral_Impl(AstBuilder*, const Location* loc, StringRef data) {
        return buildStringLiteral(*loc, data);
    }

    static Node* mkBlockStmt_Impl(AstBuilder*, const Location* loc, Node* stmts) {
        return Feather_mkLocalSpace(*loc, stmts ? all(stmts->children) : fromIniList({}));
    }
    static Node* mkIfStmt_Impl(
            AstBuilder*, const Location* loc, Node* expr, Node* thenClause, Node* elseClause) {
        return Feather_mkIf(*loc, expr, thenClause, elseClause);
    }
    static Node* mkForStmt_Impl(AstBuilder*, const Location* loc, StringRef id, Node* typeNode,
            Node* range, Node* action) {
        return mkForStmt(*loc, id, typeNode, range, action);
    }
    static Node* mkWhileStmt_Impl(
            AstBuilder*, const Location* loc, Node* expr, Node* stepAction, Node* body) {
        return Feather_mkWhile(*loc, expr, body, stepAction);
    }
    static Node* mkBreakStmt_Impl(AstBuilder*, const Location* loc) {
        return Feather_mkBreak(*loc);
    }
    static Node* mkContinueStmt_Impl(AstBuilder*, const Location* loc) {
        return Feather_mkContinue(*loc);
    }
    static Node* mkReturnStmt_Impl(AstBuilder*, const Location* loc, Node* expr) {
        return mkReturnStmt(*loc, expr);
    }
};

} // namespace SprFrontend

// Externally defined by the Sparrow code
extern "C" ParserContext* spr_parserIf_createParser(
        StringRef filename, const Location* loc, AstBuilder* astBuilder, ErrorReporter reporter);
extern "C" ParserContext* spr_parserIf_createParserFile(
        StringRef filename, const Location* loc, AstBuilder* astBuilder, ErrorReporter reporter);
extern "C" ParserContext* spr_parserIf_createParserStringRef(
        StringRef code, const Location* loc, AstBuilder* astBuilder, ErrorReporter reporter);
extern "C" void spr_parserIf_destroyParser(ParserContext* ctx);
extern "C" Node* spr_parserIf_parseModule(ParserContext* ctx);
extern "C" Node* spr_parserIf_parseExpression(ParserContext* ctx);

Parser::Parser(Location loc)
    : errReporter_(new CompilerErrorReporter)
    , astBuilder_(new CompilerAstBuilder)
    , ctx_(spr_parserIf_createParserFile(
              StringRef(loc.sourceCode->url), &loc, astBuilder_, *errReporter_)) {}

Parser::Parser(Location loc, StringRef code)
    : errReporter_(new CompilerErrorReporter)
    , astBuilder_(new CompilerAstBuilder)
    , ctx_(spr_parserIf_createParserStringRef(code, &loc, astBuilder_, *errReporter_)) {}

Parser::~Parser() {
    if (ctx_)
        spr_parserIf_destroyParser(ctx_);
}

Node* Parser::parseModule() { return ctx_ ? spr_parserIf_parseModule(ctx_) : nullptr; }

Node* Parser::parseExpression() { return ctx_ ? spr_parserIf_parseExpression(ctx_) : nullptr; }
