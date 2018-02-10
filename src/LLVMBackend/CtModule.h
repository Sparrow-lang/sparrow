#pragma once

#include "Module.h"

#include <llvm/IR/DataLayout.h>

FWD_CLASS1(llvm, ExecutionEngine);
FWD_CLASS1(llvm, Function);

namespace LLVMB {

/// Class that represents a backend CT module.
/// Uses an execution engine to implement CT evaluation
class CtModule : public Module {
public:
    explicit CtModule(const string& name);
    ~CtModule();

    /// Compile time process the given item
    void ctProcess(Node* node);

    /// If there is something to compile-time evaluate, this returns the resulting ct value in the
    /// given node If compile-time evaluation cannot be possible this returns an null node
    Node* ctEvaluate(Node* node);

    /// Register a CT API function
    /// This function will be added to the CT module, and the Sparrow programs can call it
    void ctApiRegisterFun(const char* name, void* funPtr);

public:
    virtual bool isCt() const { return true; }
    virtual void addGlobalCtor(llvm::Function* fun);
    virtual void addGlobalDtor(llvm::Function* fun);
    virtual NodeFun ctToRtTranslator() const;

private:
    //! Called after our llvm::Module was swallowed by the execution engine to create another
    //! llvm::Module.
    void recreateModule();
    //! If we have some new top-level definitions/declarations in our module, add them to the
    //! execution engine.
    void syncModule();

    void ctProcessVariable(Node* node);
    void ctProcessFunction(Node* node);
    void ctProcessClass(Node* node);
    void ctProcessBackendCode(Node* node);
    Node* ctEvaluateExpression(Node* node);

private:
    llvm::ExecutionEngine* llvmExecutionEngine_;
    //! The current LLVM module we are adding into
    //! WARNING: This may change during the compilation; i.e., for CT evaluation
    unique_ptr<llvm::Module> llvmModule_;
    llvm::DataLayout dataLayout_;
};
} // namespace LLVMB
