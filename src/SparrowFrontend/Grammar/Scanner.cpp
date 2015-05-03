#include "StdInc.h"
#include "Scanner.h"

using namespace SprFrontend;

Scanner::Scanner(istream& in)
    : SparrowFlexLexer(&in, NULL)
    , startToken_(Parser::token::END)
{
}

Scanner::Scanner(istream& in, Parser::token_type startToken)
    : SparrowFlexLexer(&in, NULL)
    , startToken_(startToken)
{
}

// virtual Parser::token_type lex(Parser::semantic_type* yylval, Parser::location_type* yylloc);
// This method is actually defined in scanner.ll.cpp generated code