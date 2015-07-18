#pragma once

FWD_CLASS1(LLVMB, Module)
FWD_CLASS2(LLVMB,Tr, TrContext)
FWD_STRUCT1(Nest, Node)

FWD_CLASS1(llvm, LLVMContext);
FWD_CLASS1(llvm, Module);
FWD_CLASS1(llvm, BasicBlock);
FWD_CLASS1(llvm, Value);
FWD_CLASS1(llvm, Function);

namespace LLVMB { namespace Tr
{
    /// Called to translate a local node; returns the llvm value corresponding to the node
    llvm::Value* translateNode(Nest::Node* node, TrContext& context);

    /// Sets the given llvm value as attribute to the node; returns the given value
    llvm::Value* setValue(Module& module, Nest::Node& node, llvm::Value* val);

    /// Gets the LLVM value associated with a node; returns null if no value is associated
    llvm::Value* getValue(Module& module, Nest::Node& node, bool doCheck = true);
}}
