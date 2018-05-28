#include "StdInc.h"
#include "Parser.h"
#include "Nodes/Builder.h"
#include "Helpers/DeclsHelpers.h"
#include "Nest/Api/SourceCode.h"

using namespace SprFrontend;

namespace SprFrontend {

struct ParserContext {}; // Don't care about the internal structure

struct SprString {
    const char* begin;
    const char* end;
    const char* endOfStore;
};

struct Token {
    Location loc;
    int type;
    SprString data;
    unsigned long long intData;
    double floatData;
};
} // namespace SprFrontend

// Externally defined by the Sparrow code
extern "C" ParserContext* spr_parserIf_createParser(
        StringRef filename, StringRef code, const Location* loc);
extern "C" void spr_parserIf_destroyParser(ParserContext* ctx);
extern "C" void spr_parserIf_nextToken(ParserContext* ctx, Token* outToken);
extern "C" Node* spr_parserIf_parseModule(ParserContext* ctx);
extern "C" Node* spr_parserIf_parseExpression(ParserContext* ctx);

// Defined here, used by the Sparrow code
extern "C" void comp_parser_reportError(Location* loc, StringRef msg) {
    Nest_reportDiagnostic(*loc, diagError, msg.begin);
}

extern "C" Node* comp_parser_addToNodeList(Node* nl, Node* newNode) {
    return Feather_addToNodeList(nl, newNode);
}

extern "C" Node* comp_parser_mkModifiers(Location* loc, Node* main, Node* mods) {
    return mkModifiers(*loc, main, mods);
}
extern "C" Node* comp_parser_mkModule(Location* loc, Node* moduleName, Node* decls) {
    return mkModule(*loc, moduleName, decls);
}
extern "C" Node* comp_parser_mkImportName(
        Location* loc, StringRef alias, Node* toImport, Node* decls) {
    return mkImportName(*loc, toImport, decls, true, alias);
}
extern "C" Node* comp_parser_mkUsing(Location* loc, StringRef alias, Node* usingNode) {
    return mkSprUsing(*loc, alias, usingNode);
}
extern "C" Node* comp_parser_mkPackage(
        Location* loc, StringRef name, Node* children, Node* params, Node* ifClause) {
    return mkSprPackage(*loc, name, children, params, ifClause);
}
extern "C" Node* comp_parser_mkDatatype(Location* loc, StringRef name, Node* params,
        Node* underlyingData, Node* ifClause, Node* children) {
    return mkSprDatatype(*loc, name, params, underlyingData, ifClause, children);
}
extern "C" Node* comp_parser_mkField(Location* loc, StringRef name, Node* typeNode, Node* init) {
    return mkSprField(*loc, name, typeNode, init);
}
extern "C" Node* comp_parser_mkConcept(
        Location* loc, StringRef name, StringRef paramName, Node* baseConcept, Node* ifClause) {
    return mkSprConcept(*loc, name, paramName, baseConcept, ifClause);
}
extern "C" Node* comp_parser_mkVar(Location* loc, StringRef name, Node* typeNode, Node* init) {
    return mkSprVariable(*loc, name, typeNode, init);
}
extern "C" Node* comp_parser_mkParameter(
        Location* loc, StringRef name, Node* typeNode, Node* init) {
    return mkSprParameter(*loc, name, typeNode, init);
}
extern "C" Node* comp_parser_mkFun(Location* loc, StringRef name, Node* formals, Node* retType,
        Node* body, Node* bodyExp, Node* ifClause) {
    if (bodyExp && !body) {
        const Location& loc2 = bodyExp->location;
        body = Feather_mkLocalSpace(*loc, fromIniList({mkReturnStmt(loc2, bodyExp)}));
        if (!retType)
            retType = mkFunApplication(loc2, mkIdentifier(loc2, fromCStr("typeOf")),
                    Feather_mkNodeList(loc2, fromIniList({bodyExp})));
    }
    return mkSprFunction(*loc, name, formals, retType, body, ifClause);
}

extern "C" Node* comp_parser_mkParenthesisExpr(Node* expr) {
    return Feather_mkNodeList(expr ? expr->location : NOLOC, fromIniList({expr}));
}
extern "C" Node* comp_parser_mkPostfixOp(Location* loc, Node* base, StringRef op) {
    return mkOperatorCall(*loc, base, op, nullptr);
}
extern "C" Node* comp_parser_mkInfixOp(Location* loc, Node* lhs, StringRef op, Node* rhs) {
    return mkInfixOp(*loc, op, lhs, rhs);
}
extern "C" Node* comp_parser_mkPrefixOp(Location* loc, StringRef op, Node* base) {
    return mkInfixOp(*loc, op, nullptr, base);
}
extern "C" Node* comp_parser_mkIdentifier(Location* loc, StringRef id) {
    return mkIdentifier(*loc, id);
}
extern "C" Node* comp_parser_mkCompoundExpr(Location* loc, Node* base, StringRef id) {
    return mkCompoundExp(*loc, base, id);
}
extern "C" Node* comp_parser_mkStarExpr(Location* loc, Node* base, StringRef id) {
    return mkStarExp(*loc, base, id);
}
extern "C" Node* comp_parser_mkDotExpr(Location* loc, Node* base, StringRef id) {
    return mkCompoundExp(*loc, base, id);
}
extern "C" Node* comp_parser_mkFunAppExpr(Location* loc, Node* base, Node* args) {
    return mkFunApplication(*loc, base, args);
}
extern "C" Node* comp_parser_mkLambdaExpr(Location* loc, Node* closureParams, Node* formals,
        Node* retType, Node* body, Node* bodyExpr) {
    return mkLambdaExp(*loc, formals, retType, body, bodyExpr, closureParams);
}
extern "C" Node* comp_parser_mkNullLiteral(Location* loc) { return buildNullLiteral(*loc); }
extern "C" Node* comp_parser_mkBoolLiteral(Location* loc, bool val) {
    return buildBoolLiteral(*loc, val);
}
extern "C" Node* comp_parser_mkIntLiteral(Location* loc, int val) {
    return buildIntLiteral(*loc, val);
}
extern "C" Node* comp_parser_mkUIntLiteral(Location* loc, unsigned int val) {
    return buildUIntLiteral(*loc, val);
}
extern "C" Node* comp_parser_mkLongLiteral(Location* loc, long long val) {
    return buildLongLiteral(*loc, val);
}
extern "C" Node* comp_parser_mkULongLiteral(Location* loc, unsigned long long val) {
    return buildULongLiteral(*loc, val);
}
extern "C" Node* comp_parser_mkFloatLiteral(Location* loc, float val) {
    return buildFloatLiteral(*loc, val);
}
extern "C" Node* comp_parser_mkDoubleLiteral(Location* loc, double val) {
    return buildDoubleLiteral(*loc, val);
}
extern "C" Node* comp_parser_mkCharLiteral(Location* loc, char val) {
    return buildCharLiteral(*loc, val);
}
extern "C" Node* comp_parser_mkStringLiteral(Location* loc, StringRef data) {
    return buildStringLiteral(*loc, data);
}

extern "C" Node* comp_parser_mkBlockStmt(Location* loc, Node* stmts) {
    return Feather_mkLocalSpace(*loc, stmts ? all(stmts->children) : fromIniList({}));
}
extern "C" Node* comp_parser_mkIfStmt(
        Location* loc, Node* expr, Node* thenClause, Node* elseClause) {
    return Feather_mkIf(*loc, expr, thenClause, elseClause);
}
extern "C" Node* comp_parser_mkForStmt(
        Location* loc, StringRef id, Node* typeNode, Node* range, Node* action) {
    return mkForStmt(*loc, id, typeNode, range, action);
}
extern "C" Node* comp_parser_mkWhileStmt(Location* loc, Node* expr, Node* stepAction, Node* body) {
    return Feather_mkWhile(*loc, expr, body, stepAction);
}
extern "C" Node* comp_parser_mkBreakStmt(Location* loc) { return Feather_mkBreak(*loc); }
extern "C" Node* comp_parser_mkContinueStmt(Location* loc) { return Feather_mkContinue(*loc); }
extern "C" Node* comp_parser_mkReturnStmt(Location* loc, Node* expr) {
    return mkReturnStmt(*loc, expr);
}

Parser::Parser(Location loc)
    : ctx_(spr_parserIf_createParser(
              fromCStr(loc.sourceCode->url), StringRef{nullptr, nullptr}, &loc)) {}

Parser::Parser(Location loc, StringRef code)
    : ctx_(spr_parserIf_createParser(StringRef{nullptr, nullptr}, code, &loc)) {}

Parser::~Parser() {
    if (ctx_)
        spr_parserIf_destroyParser(ctx_);
}

Node* Parser::parseModule() { return ctx_ ? spr_parserIf_parseModule(ctx_) : nullptr; }

Node* Parser::parseExpression() { return ctx_ ? spr_parserIf_parseExpression(ctx_) : nullptr; }
