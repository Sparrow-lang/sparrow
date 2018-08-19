#pragma once

#include "EvalMode.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Nest_Node Nest_Node;
typedef struct Nest_SourceCode Nest_SourceCode;
typedef struct Nest_Backend Nest_Backend;
typedef struct Nest_SymTab Nest_SymTab;

/// Structure describing the context in which a node is compiled
/// Each node must have such a context before it is compiled
struct Nest_CompilationContext {
    struct Nest_CompilationContext* parent; ///< The parent compilation context
    Nest_Backend* backend;                  ///< The backend (used for CT evaluation)
    Nest_SymTab* currentSymTab;             ///< The current symbol table
    EvalMode evalMode; ///< Contains the evaluation mode that is applied in the current context
    Nest_SourceCode* sourceCode; ///< The current source code in which we are compiling
};

typedef struct Nest_CompilationContext Nest_CompilationContext;

/// Create a root context for the given node. This will not have any parent context
/// By default it will not have any symtab and no source code attached to it.
Nest_CompilationContext* Nest_mkRootContext(Nest_Backend* backend, EvalMode mode);

/// Create a child context from the given context
/// If 'mode' is not modeUnspecified, it will use the given mode; otherwise it will inherit the mode
/// from the parent
Nest_CompilationContext* Nest_mkChildContext(Nest_CompilationContext* parent, EvalMode mode);

/// Create a child context from the given context
/// Always creates a SymTab, even if the given node is NULL
/// If 'mode' is not modeUnspecified, it will use the given mode; otherwise it will inherit the mode
/// from the parent
Nest_CompilationContext* Nest_mkChildContextWithSymTab(
        Nest_CompilationContext* parent, Nest_Node* symTabNode, EvalMode mode);

#ifdef __cplusplus
}
#endif
