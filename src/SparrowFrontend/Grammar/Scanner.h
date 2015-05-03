#pragma once

// Include the header of base lexer class - the template for the implementation
#ifndef __FLEX_LEXER_H
#define yyFlexLexer SparrowFlexLexer
#include "FlexLexer.h"
#undef yyFlexLexer
#endif

// Needed for 'Parser::token_type'
#include <parser.yy.hpp>

namespace SprFrontend
{
    /// The Scanner (lexer) for the Sparrow source files
    class Scanner : public SparrowFlexLexer
    {
    public:
        /// Constructs the Sparrow scanner over the given input stream
        Scanner(istream& in);
        Scanner(istream& in, Parser::token_type startToken);

        /// Main lexing function; called by Bison to fetch new tokens
        virtual Parser::token_type lex(Parser::semantic_type* yylval, Parser::location_type* yylloc);

    private:
        /// The start token; this will be artificially induced at the start of scanning
        Parser::token_type startToken_;
    };
}
