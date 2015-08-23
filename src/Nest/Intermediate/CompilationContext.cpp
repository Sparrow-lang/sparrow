#include <StdInc.h>
#include "CompilationContext.h"
#include "SymTabImpl.h"
#include <Common/Diagnostic.h>

#include <boost/bind.hpp>

using namespace Nest;

CompilationContext::CompilationContext(Backend& backend)
    : parent_(nullptr)
    , backend_(backend)
    , currentSymTab_(new SymTabImpl(nullptr, nullptr))
    , evalMode_(new EvalMode(modeRt))
    , sourceCode_(nullptr)
{
}

CompilationContext::CompilationContext(CompilationContext* src, EvalMode mode)
    : parent_(src)
    , backend_(src->backend_)
    , currentSymTab_(src->currentSymTab_)
    , evalMode_(mode == modeUnspecified ? nullptr : new EvalMode(mode))
    , sourceCode_(nullptr)
{
    // TODO (rtct): Handle this
//     if ( parent_->evalMode() == modeCt && evalMode() != modeCt )
//         REP_ERROR(NOLOC, "Cannot create non-CT context inside of a CT context");
    if ( parent_->evalMode() == modeRtCt && evalMode() == modeRt )
        REP_ERROR(NOLOC, "Cannot create RT context inside of a RTCT context");
}

CompilationContext::CompilationContext(CompilationContext* src, SymTab* curSymTab, EvalMode mode)
    : parent_(src)
    , backend_(src->backend_)
    , currentSymTab_(curSymTab)
    , evalMode_(mode == modeUnspecified ? nullptr : new EvalMode(mode))
    , sourceCode_(nullptr)
{
}

CompilationContext::~CompilationContext()
{
    delete evalMode_;
}

CompilationContext* CompilationContext::createChildContext(Node* symTabNode, EvalMode mode)
{
    return new CompilationContext(this, new SymTabImpl(currentSymTab_, symTabNode), mode);
}


SymTab* CompilationContext::currentSymTab() const
{
    return currentSymTab_;
}

EvalMode CompilationContext::evalMode() const
{
    return evalMode_ ? *evalMode_ : parent_->evalMode();
}

SourceCode* CompilationContext::sourceCode() const
{
    return sourceCode_ ? sourceCode_ : parent_->sourceCode();
}

void CompilationContext::setCurSymTab(SymTab* symTab)
{
    currentSymTab_ = symTab;
}

void CompilationContext::setEvalMode(EvalMode val)
{
    delete evalMode_;
    evalMode_ = new EvalMode(val);
}

void CompilationContext::setSourceCode(SourceCode* sourceCode)
{
    sourceCode_ = sourceCode;
}

