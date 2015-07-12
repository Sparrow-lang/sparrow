#pragma once

#include "EvalMode.h"

FWD_STRUCT1(Nest, Node);
FWD_CLASS1(Nest, SymTab);
FWD_CLASS1(Nest, Backend)
FWD_CLASS1(Nest, SourceCode)

namespace Nest
{
    /// Class that holds the context of compilation
    class CompilationContext
    {
    public:
        explicit CompilationContext(Backend& backend);
        CompilationContext(CompilationContext* src, EvalMode mode = modeUnspecified);
        CompilationContext(CompilationContext* src, SymTab* curSymTab, EvalMode mode = modeUnspecified);
        ~CompilationContext();

        /// Create a child context with a new symtab
        CompilationContext* createChildContext(Node* symTabNode, EvalMode mode = modeUnspecified);

    // State getters
    public:
        /// Getter for the backend used for CT evaluation
        Backend& backend() const { return backend_; }

        /// Getter for the current symbol table
        SymTab* currentSymTab() const;

        /// Returns the evaluation mode of this context
        EvalMode evalMode() const;

        /// Returns the current source code, in which we are compiling
        SourceCode* sourceCode() const;

    // Context modifiers
    public:
        void setCurSymTab(SymTab* symTab);
        void setEvalMode(EvalMode val);
        void setSourceCode(SourceCode* sourceCode);

    // Data
    private:
        /// The parent compilation context
        CompilationContext* parent_;

        /// The backend used for CT evaluation
        Backend& backend_;

        /// The current symbol table
        SymTab* currentSymTab_;

        /// Contains the evaluation mode that is applied in the current context
        EvalMode* evalMode_;

        /// The current source code in which we are compiling
        SourceCode* sourceCode_;
    };
}