#pragma once

#include <Nest/Intermediate/TypeRef.h>

typedef struct Nest_Location Location;

FWD_STRUCT1(Nest, Node)
FWD_CLASS1(LLVMB, Module)

FWD_CLASS1(llvm, Type);
FWD_CLASS1(llvm, LLVMContext);

namespace LLVMB { namespace Tr
{
    /// Translates the given type into a LLVM type
    llvm::Type* getLLVMType(TypeRef type, Module& module);

    /// Get the LLVM type for a native type
    /// If this is not a LLVM native type, it will return null
    llvm::Type* getNativeLLVMType(const Location& loc, const string& nativeName, llvm::LLVMContext& llvmContext);

    /// Gets the LLVM type corresponding to the given function declaration
    llvm::Type* getLLVMFunctionType(Nest::Node* funDecl, int ignoreArg, Module& module);
}}
