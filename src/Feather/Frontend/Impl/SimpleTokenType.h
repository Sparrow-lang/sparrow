#pragma once

namespace Feather
{
    /// The type of a token for a "FSimple" source code
    enum SimpleTokenType
    {
        tokenIdentifier,
        tokenString,
        tokenInteger,
        tokenLParen,
        tokenRParen,
        tokenLBrace,
        tokenRBrace,
        tokenComma,
        tokenEof,
    };
}
