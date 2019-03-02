#pragma once

#include "Nest/Utils/cppif/Type.hpp"
#include "Nest/Api/StringRef.h"

using Nest::Location;
using Nest::Node;
using Nest::StringRef;

namespace llvm {
class Type;
class LLVMContext;
} // namespace llvm

namespace LLVMB {

class Module;

namespace Tr {

struct GlobalContext;

//! Translates the given type into LLVM representation.
//! This may generate a new top-level struct in the LLVM module
llvm::Type* getLLVMType(Nest::Type type, GlobalContext& ctx);

//! Get the LLVM type for a native type
//! If this is not a LLVM native type, it will return null
llvm::Type* getNativeLLVMType(
        const Location& loc, StringRef nativeName, llvm::LLVMContext& llvmContext);

//! Gets the LLVM type corresponding to the given function declaration.
//! May create new top-level entries (struct, fun decls) in the LLVM module.
llvm::Type* getLLVMFunctionType(Node* funDecl, int ignoreArg, GlobalContext& ctx);

} // namespace Tr
} // namespace LLVMB
