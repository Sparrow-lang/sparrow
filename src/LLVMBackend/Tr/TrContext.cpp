#include <StdInc.h>
#include "TrContext.h"
#include "Scope.h"
#include "Module.h"
#include "DebugInfo.h"
#include "GlobalContext.h"

#include "Nest/Utils/Diagnostic.hpp"

using namespace LLVMB;
using namespace LLVMB::Tr;
using namespace Nest;

TrContext::TrContext(GlobalContext& ctx, llvm::BasicBlock* insertionPoint, LlvmBuilder& llvmBuilder)
    : globalContext_(ctx)
    , builder_(llvmBuilder)
    , insertionPoint_(nullptr)
    , varsBlock_(insertionPoint)
    , firstCodeBlock_(nullptr)
    , parentFun_(insertionPoint->getParent()) {
    // Leave the 'insertionPoint' block for variables; create a new block for code
    firstCodeBlock_ = llvm::BasicBlock::Create(llvmContext(), "code", parentFun_);
    insertionPoint_ = firstCodeBlock_;
}

TrContext::~TrContext() {
    // Make a jump from variables block to the first code block
    builder_.SetInsertPoint(varsBlock_);
    builder_.CreateBr(firstCodeBlock_);
}

llvm::BasicBlock* TrContext::insertionPoint() {
    // If we don't have an insertion point, create a dummy one
    if (!insertionPoint_) {
        CHECK(NOLOC, parentFun_);
        insertionPoint_ = llvm::BasicBlock::Create(
                globalContext_.targetBackend_.llvmContext(), "dumy_block", parentFun_);
        builder_.SetInsertPoint(insertionPoint_);
    }
    return insertionPoint_;
}

bool TrContext::hasValidInsertionPoint() const { return insertionPoint_ != nullptr; }

void TrContext::setInsertionPoint(llvm::BasicBlock* val) {
    if (val)
        parentFun_ = val->getParent(); // Save the parent function
    insertionPoint_ = val;
    if (val)
        builder_.SetInsertPoint(val);
}

void TrContext::ensureInsertionPoint() { builder_.SetInsertPoint(insertionPoint()); }

GlobalContext& TrContext::globalContext() const { return globalContext_; }

Module& TrContext::module() const { return globalContext_.targetBackend_; }

llvm::Function* TrContext::parentFun() const { return parentFun_; }

llvm::LLVMContext& TrContext::llvmContext() const {
    return globalContext_.targetBackend_.llvmContext();
}

llvm::Module& TrContext::llvmModule() const { return globalContext_.llvmModule_; }

LlvmBuilder& TrContext::builder() const { return builder_; }

const vector<Scope*>& TrContext::scopesStack() const { return scopesStack_; }

vector<Scope*>& TrContext::scopesStack() { return scopesStack_; }

Scope& TrContext::curScope() const {
    ASSERT(!scopesStack_.empty());
    return *scopesStack_.back();
}

Instruction& TrContext::curInstruction() const {
    ASSERT(!scopesStack_.empty());
    ASSERT(!scopesStack_.back()->instructionsStack().empty());
    return *scopesStack_.back()->instructionsStack().back();
}

llvm::AllocaInst* TrContext::addVariable(llvm::Type* type, const char* name) {
    return new llvm::AllocaInst(type, 0, name, varsBlock_);
}
