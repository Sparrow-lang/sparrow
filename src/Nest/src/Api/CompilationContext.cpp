#include "Nest/src/StdInc.hpp"
#include "Nest/Api/CompilationContext.h"
#include "Nest/Api/SymTab.h"
#include "Nest/Utils/Diagnostic.hpp"
#include "Nest/Utils/Alloc.h"

using namespace Nest;

CompilationContext* Nest_mkRootContext(Backend* backend, EvalMode mode)
{
    CompilationContext* ctx = (CompilationContext*) alloc(sizeof(CompilationContext), allocGeneral);
    ctx->parent = NULL;
    ctx->backend = backend;
    ctx->currentSymTab = Nest_mkSymTab(NULL, NULL);
    ctx->evalMode = mode;
    ctx->sourceCode = NULL;
    return ctx;
}

CompilationContext* Nest_mkChildContext(CompilationContext* parent, EvalMode mode)
{
    CompilationContext* ctx = (CompilationContext*) alloc(sizeof(CompilationContext), allocGeneral);
    ctx->parent = parent;
    ctx->backend = parent->backend;
    ctx->currentSymTab = parent->currentSymTab;
    ctx->evalMode = (mode == modeUnspecified) ? parent->evalMode : mode;
    ctx->sourceCode = parent->sourceCode;

    // TODO (rtct): Handle this
//     if ( parent->evalMode == modeCt && mode != modeCt )
//         REP_ERROR(NOLOC, "Cannot create non-CT context inside of a CT context");
    if ( parent->evalMode == modeRtCt && mode == modeRt )
        REP_ERROR(NOLOC, "Cannot create RT context inside of a RTCT context");

    return ctx;
}

CompilationContext* Nest_mkChildContextWithSymTab(CompilationContext* parent, Node* symTabNode, EvalMode mode)
{
    CompilationContext* ctx = (CompilationContext*) alloc(sizeof(CompilationContext), allocGeneral);
    ctx->parent = parent;
    ctx->backend = parent->backend;
    ctx->currentSymTab = Nest_mkSymTab(parent->currentSymTab, symTabNode);
    ctx->evalMode = (mode == modeUnspecified) ? parent->evalMode : mode;
    ctx->sourceCode = parent->sourceCode;
    return ctx;
}
