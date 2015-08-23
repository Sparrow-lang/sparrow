#pragma once

#include "EvalMode.h"

FWD_STRUCT1(Nest, Node);
FWD_CLASS1(Nest, SymTab);

typedef struct Nest_SourceCode SourceCode;
typedef struct Nest_Backend Backend;

/// Structure describing the context in which a node is compiled
/// Each node must have such a context before it is compiled
struct Nest_CompilationContext {
    struct Nest_CompilationContext* parent; ///< The parent compilation context
    Backend* backend;                       ///< The backend (used for CT evaluation)
    Nest::SymTab* currentSymTab;            ///< The current symbol table
    Nest::EvalMode* evalMode;               ///< Contains the evaluation mode that is applied in the current context
    SourceCode* sourceCode;                 ///< The current source code in which we are compiling
};

typedef struct Nest_CompilationContext Nest_CompilationContext;
typedef struct Nest_CompilationContext CompilationContext;

/// Create a root context for the given node. This will not have any parent context
/// By default it will not have any symtab and no source code attached to it.
CompilationContext* Nest_mkRootContext(Backend* backend, Nest::EvalMode mode);

/// Create a child context from the given context
/// If 'mode' is not modeUnspecified, it will use the given mode; otherwise it will inherit the mode from the parent
CompilationContext* Nest_mkChildContext(CompilationContext* parent, Nest::EvalMode mode);

/// Create a child context from the given context
/// Always creates a SymTab, even if the given node is NULL
/// If 'mode' is not modeUnspecified, it will use the given mode; otherwise it will inherit the mode from the parent
CompilationContext* Nest_mkChildContextWithSymTab(CompilationContext* parent, Nest::Node* symTabNode, Nest::EvalMode mode);

/// Gets the eval mode of this context (can be inherited from a parent context)
Nest::EvalMode Nest_getEvalMode(CompilationContext* ctx);
