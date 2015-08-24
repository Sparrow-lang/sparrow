#pragma once

FWD_CLASS1(Feather, SimpleLexer);
FWD_CLASS1(Feather, SimpleAstNode);

typedef struct Nest_CompilationContext CompilationContext;

namespace Feather
{
    /// Simple parser for the FSimple source code files
    class SimpleParser
    {
    public:
        SimpleParser(SimpleLexer& lexer);

        /// Does the parsing and returns the found content as a Nest node
        Node* parse(CompilationContext* context);

    private:
        /// The lexer used to read the tokens
        SimpleLexer& lexer_;

    private:
        /// Do the source file parsing and return the next AST node
        SimpleAstNode* parseSourceNode();

        /// Interpret a source AST node, and return a Nest node
        Node* interpret(CompilationContext* context, SimpleAstNode* srcNode);
    };
}
