#pragma once

#include <Feather/Nodes/Decls/CallConvention.h>

FWD_STRUCT1(Nest, Node)
FWD_CLASS1(LLVMB, Module)

FWD_CLASS1(llvm, Function);

namespace LLVMB { namespace Tr
{
    /// Main function used to translate a feather function in the given module
    llvm::Function* translateFunction(Nest::Node* node, Module& module);

    /// Create/translate a function that calls the given node; set 'expectsResult' to false if you don't expect any result from the function
    llvm::Function* makeFunThatCalls(Nest::Node* node, Module& module, const char* funName, bool expectsResult = false);

    /// Translate a function calling conversion; used for function calls
    llvm::CallingConv::ID translateCallingConv(Feather::CallConvention conv);
}}
