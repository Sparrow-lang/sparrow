#pragma once

#include "LlvmBuilder.h"

typedef struct Location_t Location;

FWD_CLASS1(LLVMB, Module);
FWD_CLASS2(LLVMB,Tr, Scope);
FWD_CLASS2(LLVMB,Tr, Instruction);

FWD_CLASS1(llvm, Module);
FWD_CLASS1(llvm, LLVMContext);
FWD_CLASS1(llvm, BasicBlock);
FWD_CLASS1(llvm, Function);

namespace LLVMB { namespace Tr
{
    /// Translation context structure
    /// Holds the data needed in the translation process of function bodies
    class TrContext
    {
    public:
        TrContext(Module& module, llvm::BasicBlock* insertionPoint, LlvmBuilder& llvmBuilder);
        ~TrContext();

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

        // Call this method to add a new variable to this scope; don't createa AllocaInst by hand
        llvm::AllocaInst* addVariable(llvm::Type* type, const char* name);

    private:
        /// The module for which we are making the translation; contains llvmModule and llvmContext
        Module& module_;

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
    };
}}
