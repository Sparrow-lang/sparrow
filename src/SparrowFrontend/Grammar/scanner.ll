/*********************************************************************************************************************
 * C++ Declarations
 *********************************************************************************************************************/
%{
#ifdef _WIN32
    #pragma warning(disable:4512 4127 4244 4005)
#endif

#include "Grammar/Scanner.h"

// The declaration of the lexer function
#ifndef YY_DECL
#define YY_DECL \
    SprFrontend::Parser::token_type SprFrontend::Scanner::lex( \
        SprFrontend::Parser::semantic_type* yylval, SprFrontend::Parser::location_type* yylloc)
#endif

// Types used by the lexer, imported from the parser
typedef SprFrontend::Parser::token token;
typedef SprFrontend::Parser::token_type token_type;

// Make yyterminate() return a token_type instead of int
#define yyterminate() return token::END

#define YY_NO_UNISTD_H 
%}

/*********************************************************************************************************************
 * Flex Declarations and Options
 *********************************************************************************************************************/

/* enable c++ scanner class generation */
%option c++

/* change the name of the scanner class. results in "SparrowFlexLexer" */
%option prefix="Sparrow"

/* the manual says "somewhat more optimized" */
%option batch

/* enable scanner to generate debug output. disable this for release versions. */
%option debug

/* no support for include files is planned */
%option yywrap nounput

/* enables the use of start condition stacks */
%option stack

/* The following paragraph suffices to track locations accurately. Each time
 * yylex is invoked, the begin position is moved onto the end position. */
%{
#define YY_USER_ACTION  yylloc->end.col += yyleng;
%}

/*
Symbols:
 - groupping: {} [] ()
 - separators: = : ; , . ` (note that =, : and . can be inside operators)

Keywords
 
Identifiers:
- alphanumeric (that don't start with numbers)
- operators
- alphanumeric + "_" + operators

Literals
- integer
- floating
- boolean
- null
- character
- strings
*/


Letter          [a-zA-Z_]
OpChar          [\~\!\@\#\$\%\^\&\-\+\=\|\\\:\<\>\?\/\*]
OpCharDot       {OpChar}|\.

/* don't catch comments in operator; an operator that contains a dot, must contain at least one in non-first position */
Operator        ({OpChar}*)|({OpCharDot}+\.{OpCharDot}*)
IdLetters       {Letter}({Letter}|{Digit})*
IdLettersOp     {IdLetters}\_{Operator}
Identifier      {IdLetters}|{IdLettersOp}

Digit           [0-9]
NonZeroDigit    [1-9]
BinDigit        [0-1]
HexDigit        [0-9a-fA-F]
OctalDigit      [0-7]
TetraDigit      [0-3]
DecimalNum      ({NonZeroDigit}({Digit}|[_])*([u]|[U])?([l]|[L])?)|[0]
OctalNum        [0]{OctalDigit}({OctalDigit}|[_])*([l]|[L])?
HexNum          [0]([x]|[X]){HexDigit}({HexDigit}|[_])*([l]|[L])?
BinNum          [0]([b]|[B]){BinDigit}({BinDigit}|[_])*([l]|[L])?

ExponentPart    ([e]|[E])([\+]|[\-])?{Digit}+
FloatSuffix     ([f]|[F]|[d]|[D])
Float1          {Digit}+[\.]{Digit}+{ExponentPart}?{FloatSuffix}?
Float2          [\.]{Digit}+{ExponentPart}?{FloatSuffix}?
Float3          {Digit}+{ExponentPart}{FloatSuffix}?
Float4          {Digit}+{FloatSuffix}
FloatingNum     ({Float1}|{Float2}|{Float3}|{Float4})

Escape          [\\]([r]|[n]|[b]|[f]|[t]|[\\]|[\']|[\"])
OctEscape1      [\\]{OctalDigit}
OctEscape2      [\\]{OctalDigit}{OctalDigit}
OctEscape3      [\\]{TetraDigit}{OctalDigit}{OctalDigit}
OctEscape       ({OctEscape1}|{OctEscape2}|{OctEscape3})
HexEscape1      [\\]([x]|[X]){HexDigit}
HexEscape2      [\\]([x]|[X]){HexDigit}{HexDigit}
HexEscape       ({HexEscape1}|{HexEscape2})
AnyChrChr       [^\\']
AnyStrChr       [^\\"]
Character       [\']({Escape}|{OctEscape}|{HexEscape}|{AnyChrChr})[\']
String          [\"]({Escape}|{OctEscape}|{HexEscape}|{AnyStrChr})*[\"]
StringNE        \<\{[^\}]*((\}[^]))\}\>

SingleComment   [\/][\/][^\n]*

Whitespace      [ \t\r]
Newline         [\n]


/*********************************************************************************************************************
 * Flex grammar definition
 *********************************************************************************************************************/
%%

 /* code to place at the beginning of yylex() */
%{
    // reset location
    yylloc->start = yylloc->end;

    // Fake a start token, if we received one
    if ( startToken_ != Parser::token::END )
    {
        Parser::token_type t = startToken_;
        startToken_ = Parser::token::END;
        return t;
    }
%}

{Whitespace}+ {
    yylloc->start = yylloc->end;
}

{Newline} {
    ++yylloc->end.line; yylloc->end.col = 1; yylloc->start = yylloc->end;
}

"/*"[^\*\n]* {
    char c, c1;

    while ( (c = yyinput()) != 0 )
    {
        switch ( c )
        {
        case '\n':
            ++yylloc->end.line; yylloc->end.col = 1;
            break;

        case '*':
            c1 = yyinput();
            if ( c1 == '/' )
                goto done;
            unput(c1);
            break;
        default:
            break;
        }
    }
done:
    yylloc->start = yylloc->end;
}

{SingleComment}     { yylloc->start = yylloc->end; }


 /* Symbols */
"{"         { return token::LCURLY; }
"}"         { return token::RCURLY; }
"["         { return token::LBRACKET; }
"]"         { return token::RBRACKET; }
"("         { return token::LPAREN; }
")"         { return token::RPAREN; }
":"         { return token::COLON; }
";"         { return token::SEMICOLON; }
","         { return token::COMMA; }
"."         { return token::DOT; }
"`"         { return token::BACKSQUOTE; }

 /* Operators */
"="         { return token::EQUAL; }

 /* Reserved words */
"break"         { return token::BREAK; }
"catch"         { return token::CATCH; }
"class"         { return token::CLASS; }
"concept"       { return token::CONCEPT; }
"continue"      { return token::CONTINUE; }
"datatype"      { return token::DATATYPE; }
"fun"           { return token::FUN; }
"if"            { return token::IF; }
"else"          { return token::ELSE; }
"false"         { return token::FALSE; }
"finally"       { return token::FINALLY; }
"for"           { return token::FOR; }
"import"        { return token::IMPORT; }
"null"          { return token::NULLCT; }
"package"       { return token::PACKAGE; }
"private"       { return token::PRIVATE; }
"public"        { return token::PUBLIC; }
"return"        { return token::RETURN; }
"this"          { return token::THIS; }
"throw"         { return token::THROW; }
"true"          { return token::TRUE; }
"try"           { return token::TRY; }
"using"         { return token::USING; }
"var"           { return token::VAR; }
"while"         { return token::WHILE; }

{Operator} {
    yylval->stringVal = new string(yytext, yyleng);
    return token::OPERATOR;
}

{Identifier} {
    yylval->stringVal = new string(yytext, yyleng);
    return token::IDENTIFIER;
}

{DecimalNum} {
    bool isLong = false;
    bool isUnsigned = false;

    yylval->integerVal = 0;
    for (char* p = yytext; p < yytext + yyleng; ++p) {
        if (isdigit(*p)) {
            yylval->integerVal *= 10;
            yylval->integerVal += *p - '0';
            continue;
        }
        if (*p == '_')
            continue;   
        if ((*p == 'l') || (*p == 'L'))
            isLong = true;
        else if ((*p == 'u') || (*p == 'U'))
            isUnsigned = true;
    }

    if (isLong)
        if (isUnsigned)
            return token::ULONG_LITERAL;
        else
            return token::LONG_LITERAL;
    else
        if (isUnsigned)
            return token::UINT_LITERAL;
        else
            return token::INT_LITERAL;
}

{HexNum} {
    bool isLong = false;

    yylval->integerVal = 0;
    for (char* p = yytext; p < yytext + yyleng; ++p) {
        if (isdigit(*p)) {
            yylval->integerVal *= 16;
            yylval->integerVal += *p - '0';
            continue;
        }
        else if (isxdigit(*p)) {
            yylval->integerVal *= 16;
            yylval->integerVal += tolower(*p) - 'a' + 10;
            continue;
        }
        if (*p == '_')
            continue;   
        if ((*p == 'l') || (*p == 'L'))
            isLong = true;
    }

    if (isLong)
        return token::LONG_LITERAL;
    else
        return token::INT_LITERAL;
}

{OctalNum} {
    bool isLong = false;

    yylval->integerVal = 0;
    for (char* p = yytext; p < yytext + yyleng; ++p) {
        if (isdigit(*p)) {
            yylval->integerVal *= 8;
            yylval->integerVal += *p - '0';
            continue;           
        }
        if (*p == '_')
            continue;
        if ((*p == 'l') || (*p == 'L'))
            isLong = true;
    }

    if (isLong)
        return token::LONG_LITERAL;
    else
        return token::INT_LITERAL;
}

{BinNum} {
    bool isLong = false;

    yylval->integerVal = 0;
    for (char* p = yytext; p < yytext + yyleng; ++p) {
        if (isdigit(*p)) {
            yylval->integerVal *= 2;
            yylval->integerVal += *p - '0';
            continue;           
        }
        if (*p == '_')
            continue;
        if ((*p == 'l') || (*p == 'L'))
            isLong = true;
    }

    if (isLong)
        return token::LONG_LITERAL;
    else
        return token::INT_LITERAL;
}

{FloatingNum} {
    bool isDouble = true;
    char* p = &yytext[yyleng-1];
    int len;
    char buff[64];

    if ((*p == 'f') || (*p == 'F')) {
        isDouble = false;
        --p;
    }
    len = p - yytext + 1;
    memcpy(buff, yytext, len);
    buff[len] = '\0';
    yylval->floatingVal = atof(buff);

    if (isDouble)
        return token::DOUBLE_LITERAL;
    else
        return token::FLOAT_LITERAL;

}

{Character} {
    char* p = &yytext[1];

    if (*p == '\\') {
        ++p;
        switch (*p) {
            case 'r':
                yylval->charVal = '\r';
                break;
            case 'n':
                yylval->charVal = '\n';
                break;
            case 'b':
                yylval->charVal = '\b';
                break;
            case 'f':
                yylval->charVal = '\f';
                break;
            case 't':
                yylval->charVal = '\t';
                break;
            case '\\':
                yylval->charVal = '\\';
                break;
            case '\'':
                yylval->charVal = '\'';
                break;
            case '\"':
                yylval->charVal = '\"';
                break;
            case 'x':
            case 'X':
                ++p;
                if (isdigit(*p))
                    yylval->charVal = *p - '0';
                else
                    yylval->charVal = tolower(*p) - 'a' + 10;
                ++p;
                if (isdigit(*p)) {
                    yylval->charVal *= 16;
                    yylval->charVal += *p - '0';
                }
                else if (isxdigit(*p)) {
                    yylval->charVal *= 16;
                    yylval->charVal += tolower(*p) - 'a' + 10;
                }
                break;
            default:
                yylval->charVal = *p - '0';
                ++p;
                if ((*p >= '0') && (*p <= '7')) {
                    yylval->charVal *= 8;
                    yylval->charVal += *p - '0';
                }
                ++p;
                if ((*p >= '0') && (*p <= '7')) {
                    yylval->charVal *= 8;
                    yylval->charVal += *p - '0';
                }
        }
    }
    else
        yylval->charVal = *p;

    return token::CHAR_LITERAL;
}

{String} {
    char* p = &yytext[1];
    yylval->stringVal = new string;
    yylval->stringVal->reserve(yyleng);
    char c;
    
    while (*p != '\"') {
        if (*p == '\\') {
            ++p;
            switch (*p) {
                case 'r':
                    yylval->stringVal->push_back('\r');
                    break;
                case 'n':
                    yylval->stringVal->push_back('\n');
                    break;
                case 'b':
                    yylval->stringVal->push_back('\b');
                    break;
                case 'f':
                    yylval->stringVal->push_back('\f');
                    break;
                case 't':
                    yylval->stringVal->push_back('\t');
                    break;
                case '\\':
                    yylval->stringVal->push_back('\\');
                    break;
                case '\'':
                    yylval->stringVal->push_back('\'');
                    break;
                case '\"':
                    yylval->stringVal->push_back('\"');
                    break;
                case 'x':
                case 'X':
                    ++p;
                    if (isdigit(*p))
                        c = *p - '0';
                    else
                        c = tolower(*p) - 'a' + 10;
                    ++p;
                    if (isdigit(*p)) {
                        c *= 16;
                        c += *p - '0';
                    }
                    else if (isxdigit(*p)) {
                        c *= 16;
                        c += tolower(*p) - 'a' + 10;
                    }
                    else
                        --p;
                    yylval->stringVal->push_back(c);
                    break;
                default:
                    c = *p - '0';
                    ++p;
                    if ((*p >= '0') && (*p <= '7')) {
                        c *= 8;
                        c += *p - '0';
                    }
                    else {
                        --p;
                        goto octal_done1;
                    }
                    ++p;
                    if ((*p >= '0') && (*p <= '7')) {
                        c *= 8;
                        c += *p - '0';
                    }
                    else
                        --p;
octal_done1:
                    yylval->stringVal->push_back(c);
            }
        }
        else
        {
            if (*p == '\n')
                ++yylloc->end.line; yylloc->end.col = 1;
            yylval->stringVal->push_back(*p);
        }
        ++p;
    }   

    return token::STRING_LITERAL;
}

"<{"[^\}\n]* {
    char c, c1;
    char* p = &yytext[2];
    yylval->stringVal = new string;
    yylval->stringVal->assign(p, p+yyleng-2);
    
    while ( (c = yyinput()) != 0 )
    {
        switch ( c )
        {
        case '\n':
            ++yylloc->end.line; yylloc->end.col = 1;
            break;
            
        case '}':
            c1 = yyinput();
            if ( c1 == '>' )
                goto done2;
            unput(c1);
            break;
        default:
            break;
        }
        yylval->stringVal->push_back(c);
    }
done2:
    yylloc->start = yylloc->end;
    
    return token::STRING_LITERAL;
}



 /* pass all other characters up to bison */
. {
    return static_cast<token_type>(*yytext);
}

%%
/*********************************************************************************************************************
 * Additional C++ code
 *********************************************************************************************************************/

#ifdef yylex
#undef yylex
#endif

int SparrowFlexLexer::yylex()
{
    cerr << "in SparrowFlexLexer::yylex() !" << endl;
    return 0;
}

int SparrowFlexLexer::yywrap()
{
    // We don't support wrapping to other files when scanning the current files encounters EOF
    // Just terminate the scanning
    return 1;
}
