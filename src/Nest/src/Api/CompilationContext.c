#include "Nest/Api/CompilationContext.h"
#include "Nest/Api/SymTab.h"
#include "Nest/Utils/Diagnostic.h"
#include "Nest/Utils/Alloc.h"

CompilationContext* Nest_mkRootContext(Backend* backend, EvalMode mode) {
    CompilationContext* ctx = (CompilationContext*)alloc(sizeof(CompilationContext), allocGeneral);
    ctx->parent = 0;
    ctx->backend = backend;
    ctx->currentSymTab = Nest_mkSymTab(0, 0);
    ctx->evalMode = mode;
    ctx->sourceCode = 0;
    return ctx;
}

CompilationContext* Nest_mkChildContext(CompilationContext* parent, EvalMode mode) {
    CompilationContext* ctx = (CompilationContext*)alloc(sizeof(CompilationContext), allocGeneral);
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

CompilationContext* Nest_mkChildContextWithSymTab(
        CompilationContext* parent, Node* symTabNode, EvalMode mode) {
    CompilationContext* ctx = (CompilationContext*)alloc(sizeof(CompilationContext), allocGeneral);
    ctx->parent = parent;
    ctx->backend = parent->backend;
    ctx->currentSymTab = Nest_mkSymTab(parent->currentSymTab, symTabNode);
    ctx->evalMode = (mode == modeUnspecified) ? parent->evalMode : mode;
    ctx->sourceCode = parent->sourceCode;
    return ctx;
}
