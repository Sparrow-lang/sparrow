#pragma once

#include "Feather/Api/Feather.h"

namespace llvm {
class Function;
}

namespace LLVMB {

namespace Tr {

struct GlobalContext;
using Nest::Node;

//! Main function used to translate a feather function into LLVM IR.
//! It will translate the definition of the function, if not already translated.
llvm::Function* translateFunction(Node* node, GlobalContext& ctx);

//! Create/translate a function that calls the given node.
//!
//! If 'expectsResult' is set, we add the type of the node as a pointer param to the function, and
//! we make the function store the result there. If false, this will be a void() function.
//!
//! The definition of this will be placed in 'ctx.llvmModule_'. All the other referenced definitions
//! will be placed in 'ctx.definitionsLlvmModule_'.
llvm::Function* makeFunThatCalls(
        Node* node, GlobalContext& ctx, const char* funName, bool expectsResult = false);

//! Translate a function calling conversion; used for function calls
llvm::CallingConv::ID translateCallingConv(CallConvention conv);

} // namespace Tr
} // namespace LLVMB
