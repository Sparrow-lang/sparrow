#include <StdInc.h>
#include "SimpleLexer.h"

#include <Nest/Common/Diagnostic.h>

using namespace Feather;
using namespace Nest::Common;
using namespace Nest;

SimpleLexer::SimpleLexer(const Nest::SourceCode& sourceCode, const char* sourceBuffer)
    : origSource_(sourceBuffer)
    , source_(nullptr)
    , curToken_(Location(), tokenEof)
    , location_(sourceCode)
{
}

SimpleToken SimpleLexer::consumeToken()
{
    if ( source_ == nullptr )
    {
        source_ = origSource_.c_str();
        nextToken();
    }
    SimpleToken t = curToken_;
    nextToken();
    return t;
}

SimpleToken SimpleLexer::peekToken()
{
    if ( source_ == nullptr )
    {
        source_ = origSource_.c_str();
        nextToken();
    }
    return curToken_;
}

const Location& SimpleLexer::curLocation() const
{
    return location_;
}

string SimpleLexer::getSourceCodeLine(int lineNo) const
{
    const char* p = origSource_.c_str();
    const char* oldLineStart = p;
    int l = 0;
    while ( *p != 0 )
    {
        if ( !consumeNewLineImpl(p) )
            ++p;
        else
        {
            ++l;
            if ( l == lineNo )
                return string(oldLineStart, p);
            oldLineStart = p;
        }
    }
    return string();
}


void SimpleLexer::nextToken()
{
    // Consume all the characters to be ignored
    consumeIgnoreChars();

    // Start the location range from the current point
    location_.step();

    // Check for EOF
    if ( *source_ == 0 )
    {
        curToken_ = SimpleToken(location_, tokenEof);
        return;
    }

    // Check for symbols
    if ( *source_ == '(' )  { nextChar(); curToken_ = SimpleToken(location_, tokenLParen); return; }
    if ( *source_ == ')' )  { nextChar(); curToken_ = SimpleToken(location_, tokenRParen); return; }
    if ( *source_ == '{' )  { nextChar(); curToken_ = SimpleToken(location_, tokenLBrace); return; }
    if ( *source_ == '}' )  { nextChar(); curToken_ = SimpleToken(location_, tokenRBrace); return; }
    if ( *source_ == ',' )  { nextChar(); curToken_ = SimpleToken(location_, tokenComma); return; }

    // Check for strings
    if ( *source_ == '"' )
    {
        nextChar(); // Skip the starting quote
        const char* stringStart = source_;
        const char* stringEnd = consumeString();
        curToken_ = SimpleToken(location_, tokenString, stringStart, stringEnd);
        return;
    }

    // Check for numbers
    if ( isdigit(*source_) )
    {
        const char* numStart = source_;
        for ( ; isdigit(*source_); nextChar() )
            ;
        curToken_ = SimpleToken(location_, tokenInteger, numStart, source_);
        return;
    }

    // Check for identifiers
    if ( isalpha(*source_) )
    {
        const char* identStart = source_;
        for ( ; isalnum(*source_); nextChar() )
            ;
        curToken_ = SimpleToken(location_, tokenIdentifier, identStart, source_);
        return;
    }

    char ch = *source_;
    nextChar();
    REP_ERROR(location_, "Invalid character found: '%1%'") % ch;
    curToken_ = SimpleToken(location_, tokenEof);
}

void SimpleLexer::nextChar()
{
    location_.addColumns(1);
    ++source_;
}

void SimpleLexer::consumeIgnoreChars()
{
    // Consume all the characters to be ignored
    for ( ;; )
    {
        // Check for new lines
        if ( consumeNewLine() )
            continue;

        // Check for whitespaces
        if ( isspace(*source_) )
        {
            nextChar();
            continue;
        }

        // Check for single line comments
        if ( source_[0] == '/' && source_[1] == '/' )
        {
            while ( *source_ && !consumeNewLine() )
                nextChar();
            continue;
        }

        // Check for multi line comments
        if ( source_[0] == '/' && source_[1] == '*' )
        {
            while ( source_[0] != 0 && !(source_[0] == '*' && source_[1] == '/') )
            {
                if ( consumeNewLine() )
                    ;
                else
                    nextChar();
            }
            if ( source_[0] != 0 )
            {
                nextChar();
                nextChar();
            }
            continue;
        }

        // Other character found, break from the loop
        break;
    }
}


bool SimpleLexer::consumeNewLine()
{
    if ( consumeNewLineImpl(source_) )
    {
        location_.addLines(1);
        return true;
    }
    else
        return false;
}

bool SimpleLexer::consumeNewLineImpl(const char*& p)
{
    if ( (p[0] == '\n' && p[1] == '\r')
        || (p[0] == '\r' && p[1] == '\n') )
    {
        p += 2;
        return true;
    }
    if ( p[0] == '\n' || p[0] == '\r' )
    {
        p += 1;
        return true;
    }
    return false;
}

const char* SimpleLexer::consumeString()
{
    for ( ;; )
    {
        // Check for new-lines; consume them
        while ( consumeNewLine() )
            ;

        // Check for the ending quotes
        if ( *source_ == '"' && *(source_-1) != '\\' )
        {
            const char* quoteChar = source_;
            // Move the cursor past the ending quote
            nextChar();
            return quoteChar;
        }

        // Check for EOF
        if ( *source_ == 0 )
        {
            REP_ERROR(curLocation(), "End-of-file reached inside string literal");
        }

        // Move to the next character
        nextChar();
    }
}

