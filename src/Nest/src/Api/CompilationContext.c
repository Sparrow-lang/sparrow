#include "Nest/Api/CompilationContext.h"
#include "Nest/Api/SymTab.h"
#include "Nest/Utils/Diagnostic.h"
#include "Nest/Utils/Alloc.h"

Nest_CompilationContext* Nest_mkRootContext(Nest_Backend* backend, EvalMode mode) {
    Nest_CompilationContext* ctx = (Nest_CompilationContext*)alloc(sizeof(Nest_CompilationContext), allocGeneral);
    ctx->parent = 0;
    ctx->backend = backend;
    ctx->currentSymTab = Nest_mkSymTab(0, 0);
    ctx->evalMode = mode;
    ctx->sourceCode = 0;
    return ctx;
}

Nest_CompilationContext* Nest_mkChildContext(Nest_CompilationContext* parent, EvalMode mode) {
    Nest_CompilationContext* ctx = (Nest_CompilationContext*)alloc(sizeof(Nest_CompilationContext), allocGeneral);
    ctx->parent = parent;
    ctx->backend = parent->backend;
    ctx->currentSymTab = parent->currentSymTab;
    ctx->evalMode = (mode == modeUnspecified) ? parent->evalMode : mode;
    ctx->sourceCode = parent->sourceCode;

    if (parent->evalMode == modeCt && mode != modeCt)
        Nest_reportDiagnostic(
                NOLOC, diagError, "Cannot create non-CT context inside of a CT context");

    return ctx;
}

Nest_CompilationContext* Nest_mkChildContextWithSymTab(
        Nest_CompilationContext* parent, Nest_Node* symTabNode, EvalMode mode) {
    Nest_CompilationContext* ctx = (Nest_CompilationContext*)alloc(sizeof(Nest_CompilationContext), allocGeneral);
    ctx->parent = parent;
    ctx->backend = parent->backend;
    ctx->currentSymTab = Nest_mkSymTab(parent->currentSymTab, symTabNode);
    ctx->evalMode = (mode == modeUnspecified) ? parent->evalMode : mode;
    ctx->sourceCode = parent->sourceCode;
    return ctx;
}
