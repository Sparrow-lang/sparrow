#pragma once

#include <Feather/Nodes/Decls/Function.h>

FWD_CLASS1(LLVMB, Module)
FWD_CLASS1(Nest, DynNode);
FWD_CLASS1(Feather, Function);

FWD_CLASS1(llvm, Function);

namespace LLVMB { namespace Tr
{
    /// Main function used to translate a feather function in the given module
    llvm::Function* translateFunction(Feather::Function* node, Module& module);

    /// Create/translate a function that calls the given node; set 'expectsResult' to false if you don't expect any result from the function
    llvm::Function* makeFunThatCalls(Nest::DynNode* node, Module& module, const char* funName, bool expectsResult = false);

    /// Translate a function calling conversion; used for function calls
    llvm::CallingConv::ID translateCallingConv(Feather::CallConvention conv);
}}
