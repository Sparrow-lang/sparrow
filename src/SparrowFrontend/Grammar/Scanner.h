#pragma once

// Needed for 'Parser::token_type'
#include <parser.yy.hpp>

namespace SprFrontend
{
    struct ScannerContext;
    struct Token;

    /// The Scanner (lexer) for the Sparrow source files
    class Scanner //: public SparrowFlexLexer
    {
    public:
        /// Constructs the Sparrow scanner over the file indicated by the given source code
        Scanner(Location loc, Parser::token_type startToken = Parser::token::END);

        /// Constructs the Sparrow scanner over the file indicated by the given source code, parsing the given string
        Scanner(Location loc, StringRef code, Parser::token_type startToken = Parser::token::END);

        ~Scanner();

        /// Main lexing function; called by Bison to fetch new tokens
        Parser::token_type lex(Parser::semantic_type* yylval, Parser::location_type* yylloc);

    private:
        /// The context of the scanner; used for interop with the code in Sparrow
        ScannerContext* ctx_;

        /// Token data used for lexing
        Token* workTokenData_;

        /// The start token; this will be artificially induced at the start of scanning
        Parser::token_type startToken_;
    };
}
