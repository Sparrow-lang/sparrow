#pragma once

namespace SprFrontend {
struct ParserContext;
struct Token;

/// The parser for the Sparrow source files
class Parser {
public:
    /// Constructs the Sparrow parser over the file indicated by the given source code
    Parser(Location loc);

    /// Constructs the Sparrow parser over the file indicated by the given source code, parsing the
    /// given string
    Parser(Location loc, StringRef code);

    ~Parser();

    Node* parseModule();
    Node* parseExpression();

private:
    /// The context of the Parser; used for interop with the code in Sparrow
    ParserContext* ctx_;
};
} // namespace SprFrontend
