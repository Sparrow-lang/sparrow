#pragma once

namespace SprFrontend {
struct ParserContext;
struct CompilerErrorReporter;
struct CompilerAstBuilder;

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
    //! The error reporter passed to the Sparrow parser code
    CompilerErrorReporter* errReporter_;
    //! The AST builder object passed to the Sparrow parser code
    CompilerAstBuilder* astBuilder_;
    //! The context of the Parser; used for interop with the code in Sparrow
    ParserContext* ctx_;
};
} // namespace SprFrontend
