#pragma once

#include "Nest/Api/TypeRef.h"

#include <unordered_map>

FWD_CLASS1(llvm, LLVMContext);
FWD_CLASS1(llvm, Module);

using Nest::TypeRef;

namespace LLVMB {
/// Class that is responsible for computing the sizeOf and alignOf data types
class DataLayoutHelper {
public:
    DataLayoutHelper();
    ~DataLayoutHelper();

    /// Get the size of the given type in bytes, according to the current data layout
    size_t getSizeOf(TypeRef type);

    /// Get the alignment of the given type in bytes, according to the current data layout
    size_t getAlignOf(TypeRef type);

private:
    unique_ptr<llvm::LLVMContext> llvmContext_;
    unique_ptr<llvm::Module> llvmModule_;

    unordered_map<TypeRef, size_t> sizesOfTypes_;
    unordered_map<TypeRef, size_t> alignmentsOfTypes_;
};

} // namespace LLVMB
