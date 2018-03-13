#pragma once

#include "LlvmBuilder.h"

typedef struct Nest_Location Location;
typedef struct Nest_Node Node;

FWD_CLASS1(LLVMB, Module);
FWD_CLASS2(LLVMB, Tr, Scope);
FWD_CLASS2(LLVMB, Tr, Instruction);
FWD_STRUCT2(LLVMB, Tr, GlobalContext);

FWD_CLASS1(llvm, Module);
FWD_CLASS1(llvm, LLVMContext);
FWD_CLASS1(llvm, BasicBlock);
FWD_CLASS1(llvm, Function);

namespace LLVMB {
namespace Tr {
// TODO (backend): rename this to LocalContext

/// Translation context structure
/// Holds the data needed in the translation process of function bodies
class TrContext {
public:
    TrContext(GlobalContext& ctx, llvm::BasicBlock* insertionPoint, LlvmBuilder& llvmBuilder);
    ~TrContext();

    /// Getter for the global context in which we operate
    GlobalContext& globalContext() const;

    /// Getter for the module for which we are doing the translations
    Module& module() const;

    /// Getter for the LLVM context used to add new instructions
    llvm::LLVMContext& llvmContext() const;

    /// Getter for the LLVM module into which we add new instructions
    llvm::Module& llvmModule() const;

    LlvmBuilder& builder() const;

    /// Getter for the parent function
    llvm::Function* parentFun() const;

    /// Getter for the current insertion point
    /// If no insertion point is defined, this will create a dummy insertion point
    llvm::BasicBlock* insertionPoint();

    /// Check if we have a valid, non-dummy insertion point active
    bool hasValidInsertionPoint() const;

    /// Set the current insertion point
    /// To disable insertion after a terminating instruction, pass nullptr to this method
    void setInsertionPoint(llvm::BasicBlock* val);

    /// Ensures that we have a valid insertion point to create new instructions in
    void ensureInsertionPoint();

    /// Getter for the current scopes stack
    const vector<Scope*>& scopesStack() const;
    vector<Scope*>& scopesStack();

    /// Returns the current scope; assumes that we have one valid
    Scope& curScope() const;

    /// Returns the current instruction from the current scope; assumes that we have one valid
    Instruction& curInstruction() const;

    // Call this method to add a new variable to this scope; don't create a AllocaInst by hand
    llvm::AllocaInst* addVariable(llvm::Type* type, const char* name);

private:
    /// The global context in which we are operating
    GlobalContext& globalContext_;

    /// The builder object used to build LLVM constructs
    LlvmBuilder& builder_;

    /// The current insertion point, where new code should be inserted
    llvm::BasicBlock* insertionPoint_;

    /// The block where the variables lie
    llvm::BasicBlock* varsBlock_;

    /// The first block of code
    llvm::BasicBlock* firstCodeBlock_;

    /// The current function in which we are inserting
    llvm::Function* parentFun_;

    /// The stacks of scopes until the current instruction that are in translations
    vector<Scope*> scopesStack_;

public:
    //! The resulting LLVM values for local nodes
    unordered_map<Node*, llvm::Value*> resultingValues_;

    //! Instruction guards for the while nodes
    unordered_map<Node*, Tr::Scope*> whileInstrGuards_;
    //! The 'step' label for the while nodes
    unordered_map<Node*, llvm::BasicBlock*> whileStepLabels_;
    //! The 'end' label for the while nodes
    unordered_map<Node*, llvm::BasicBlock*> whileEndLabels_;
};
} // namespace Tr
} // namespace LLVMB
