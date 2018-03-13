#pragma once

#include <string>
#include <vector>

FWD_CLASS1(llvm, Module);

namespace LLVMB {
//! Generate LLVM assembly file for the given run-time module
void generateRtAssembly(const llvm::Module& module);

//! Generate LLVM assembly for the given CT module.
//! Because during the compilation time we will process multiple LLVM modules, this function
//! will be called multiple times, for each such module. Instead of writing the CT assembly to a
//! single file, we instead write a lot of .ll files in a directory.
void generateCtAssembly(const llvm::Module& module);

/// Link the given input module to produce a final binary, as native executable
void link(const vector<llvm::Module*>& inputs, const string& outFilename);
} // namespace LLVMB
