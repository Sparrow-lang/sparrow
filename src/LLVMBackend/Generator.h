#pragma once

#include <string>
#include <vector>

FWD_CLASS1(llvm, Module);

namespace LLVMB
{
    /// Generate llvm assembly file for the given module
    void generateAssembly(const llvm::Module& module, const string& outFilename, const string& ext);

    /// Link the given input module to produce a final binary, as native executable
    void link(const vector<llvm::Module*>& inputs, const string& outFilename);
}
