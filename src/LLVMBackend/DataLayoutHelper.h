#pragma once

#include "Nest/Utils/cppif/Type.hpp"

#include <unordered_map>

FWD_CLASS1(llvm, LLVMContext);
FWD_CLASS1(llvm, Module);

namespace LLVMB {

using Nest::Type;

/// Class that is responsible for computing the sizeOf and alignOf data types
class DataLayoutHelper {
public:
    DataLayoutHelper();
    ~DataLayoutHelper();

    /// Get the size of the given type in bytes, according to the current data layout
    size_t getSizeOf(Type type);

    /// Get the alignment of the given type in bytes, according to the current data layout
    size_t getAlignOf(Type type);

private:
    unique_ptr<llvm::LLVMContext> llvmContext_;
    unique_ptr<llvm::Module> llvmModule_;

    unordered_map<Type, size_t> sizesOfTypes_;
    unordered_map<Type, size_t> alignmentsOfTypes_;
};

} // namespace LLVMB
