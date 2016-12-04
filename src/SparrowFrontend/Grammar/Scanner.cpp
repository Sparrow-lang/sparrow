#include "StdInc.h"
#include "Scanner.h"
#include "Nest/Api/SourceCode.h"

using namespace SprFrontend;

namespace SprFrontend {

    struct ScannerContext{}; // Don't care about the internal structure

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
}

// Externally defined by the Sparrow code
extern "C" ScannerContext* spr_scannerIf_createScanner(StringRef filename, StringRef code, const Location* loc);
extern "C" void spr_scannerIf_destroyScanner(ScannerContext* ctx);
extern "C" void spr_scannerIf_nextToken(ScannerContext* ctx, Token* outToken);

// Defined here, used by the Sparrow code
extern "C" void comp_scannerIf_reportError(Location loc, StringRef msg) {
    Nest_reportDiagnostic(loc, diagError, msg.begin);
}

Scanner::Scanner(Location loc, Parser::token_type startToken)
    : ctx_(spr_scannerIf_createScanner(fromCStr(loc.sourceCode->url), StringRef{nullptr, nullptr}, &loc))
    , workTokenData_(new Token)
    , startToken_(startToken)
{
    memset(workTokenData_, 0, sizeof(Token));
}

Scanner::Scanner(Location loc, StringRef code, Parser::token_type startToken)
    : ctx_(spr_scannerIf_createScanner(StringRef{nullptr, nullptr}, code, &loc))
    , workTokenData_(new Token)
    , startToken_(startToken)
{
    memset(workTokenData_, 0, sizeof(Token));
}

Scanner::~Scanner()
{
    if ( ctx_ )
        spr_scannerIf_destroyScanner(ctx_);
    delete workTokenData_;
}

Parser::token_type Scanner::lex(Parser::semantic_type* yylval, Parser::location_type* yylloc) {
    // Fake a start token, if we received one
    if ( startToken_ != Parser::token::END )
    {
        Parser::token_type t = startToken_;
        startToken_ = Parser::token::END;
        return t;
    }

    // Invoke the Sparrow code to get the next token
    // Read the next token
    spr_scannerIf_nextToken(ctx_, workTokenData_);

    // Set the proper location
    *yylloc = workTokenData_->loc;

    // Output the content
    // cout << workTokenData_->loc << ": " << workTokenData_->type << " - '" << string(workTokenData_->data.begin, workTokenData_->data.end) << "'" << endl;

    switch ( workTokenData_->type ) {
    case Parser::token::INT_LITERAL:
        yylval->integerVal = workTokenData_->intData;
        break;
    case Parser::token::LONG_LITERAL:
        yylval->integerVal = workTokenData_->intData;
        break;
    case Parser::token::UINT_LITERAL:
        yylval->integerVal = workTokenData_->intData;
        break;
    case Parser::token::ULONG_LITERAL:
        yylval->integerVal = workTokenData_->intData;
        break;
    case Parser::token::FLOAT_LITERAL:
        yylval->floatingVal = workTokenData_->floatData;
        break;
    case Parser::token::DOUBLE_LITERAL:
        yylval->floatingVal = workTokenData_->floatData;
        break;
    case Parser::token::CHAR_LITERAL:
        yylval->charVal = *workTokenData_->data.begin;
        break;
    case Parser::token::STRING_LITERAL:
    case Parser::token::IDENTIFIER:
    case Parser::token::OPERATOR:
        yylval->stringVal = new string(workTokenData_->data.begin, workTokenData_->data.end);
        break;
    default:
        break;
    }

    return Parser::token_type(workTokenData_->type);
}
