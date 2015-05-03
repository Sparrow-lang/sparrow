#pragma once

#ifdef _MSC_VER
#pragma warning(push,1)
#endif
#include "llvm/IR/IRBuilder.h"
#ifdef _MSC_VER
#pragma warning(pop)
#endif

namespace LLVMB { namespace Tr
{
    typedef llvm::IRBuilder<> LlvmBuilder;
}}