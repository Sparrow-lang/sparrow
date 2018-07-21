#pragma once

FWD_CLASS1(LLVMB, Module)
FWD_CLASS2(LLVMB, Tr, TrContext)

FWD_CLASS1(llvm, LLVMContext);
FWD_CLASS1(llvm, Module);
FWD_CLASS1(llvm, BasicBlock);
FWD_CLASS1(llvm, Value);
FWD_CLASS1(llvm, Function);

namespace LLVMB {
namespace Tr {

using Nest::Node;

/// Called to translate a local node; returns the llvm value corresponding to the node
llvm::Value* translateNode(Node* node, TrContext& context);

/// Sets the given llvm value as attribute to the node; returns the given value
llvm::Value* setValue(TrContext& context, Node& node, llvm::Value* val);

/// Gets the LLVM value associated with a node; returns null if no value is associated
llvm::Value* getValue(TrContext& context, Node& node, bool doCheck = true);
} // namespace Tr
} // namespace LLVMB
