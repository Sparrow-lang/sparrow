#pragma once

#include "Module.h"

FWD_CLASS1(Feather, Var);
FWD_CLASS1(Feather, Class);

FWD_CLASS1(llvm, ExecutionEngine);
FWD_CLASS1(llvm, Function);

namespace LLVMB { namespace Tr
{
    /// Class that represents a backend CT module.
    /// Uses an execution engine to implement CT evaluation
    class CtModule : public Module
    {
    public:
        explicit CtModule(const string& name);
        ~CtModule();

        /// Compile time process the given item
        void ctProcess(Node* node);

        /// If there is something to compile-time evaluate, this returns the resulting ct value in the given node
        /// If compile-time evaluation cannot be possible this returns an null node
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
		void ctProcessVariable(Feather::Var* node);
		void ctProcessFunction(Node* node);
		void ctProcessClass(Feather::Class* node);
		void ctProcessBackendCode(Node* node);
		Node* ctEvaluateExpression(Node* node);

    private:
	    llvm::ExecutionEngine* llvmExecutionEngine_;
    };
}}
