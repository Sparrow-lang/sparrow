#pragma once

#include "SimpleToken.h"

typedef struct Nest_SourceCode SourceCode;

namespace Feather
{
    /// Simple lexer to handle FSimple source code files
    class SimpleLexer
    {
    public:
        SimpleLexer(const SourceCode& sourceCode, const char* sourceBuffer);

        /// Gets the new token from the input source - the token is consumed
        SimpleToken consumeToken();

        /// Retrieve the next token from the input source without consuming it
        SimpleToken peekToken();

        /// Gets the current location
        const Location& curLocation() const;

        /// Method called from the source code to get the line for a given line number
        /// It doesn't start from the current position, as it ises the whole file content to get the line
        string getSourceCodeLine(int lineNo) const;

    private:
        /// Pointer to the source buffer; the pointer will NOT be changed while consuming tokens
        string origSource_;

        /// Pointer to the source buffer; the pointer will move towards end as we will consume tokens
        const char* source_;

        /// The current token in the source file
        SimpleToken curToken_;

        /// The current location
        Location location_;

    private:
        /// Moves to the next token in the source file
        void nextToken();

        /// Move to the next character
        void nextChar();

        /// Consume all the characters to be ignored (whitespaces, new-lines, comments)
        void consumeIgnoreChars();

        /// Tests if the given string starts with a newline; if yes, consumes the new-line chars from the source
        bool consumeNewLine();
        static bool consumeNewLineImpl(const char*& p);

        /// Assuming we are at the start of a string literal, consume until the end of the string literal
        /// Returns pointer to the character of the ending quote (after the last char in the literal)
        const char* consumeString();
    };
}
