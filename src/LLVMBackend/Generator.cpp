#include <StdInc.h>
#include "Generator.h"

#include "Nest/Utils/Diagnostic.hpp"
#include "Nest/Utils/PrintTimer.hpp"
#include "Nest/Api/Compiler.h"
#include "Nest/Utils/CompilerSettings.hpp"

#include <boost/filesystem.hpp>

#include <algorithm>
#include <iomanip>

#include <llvm/Linker/Linker.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Bitcode/BitcodeWriter.h>
#include <llvm/Support/FileUtilities.h>
#include <llvm/Support/Program.h>
#include <llvm/Support/ToolOutputFile.h>

using namespace LLVMB;
using namespace llvm;
using namespace llvm::sys;

namespace
{
    /// Replace the extension of 'origFilename' with the given extension.
    /// If the result is the output filename, it will add some characters to make it different
    string replaceExtension(const string& outputFilename, string origFilename, const string& extension) {
        string res;
        boost::filesystem::path f = origFilename;
        while ( true ) {
            f.replace_extension(extension);
            res = f.string();

            // If we obtained a file that is different form the output file, return it
            if ( res != outputFilename )
                return res;

            // Add a one to the filename stem to get a different result
            f = f.stem();
            f += "1";
        }
    }
    /// Run the command with the given arguments
    /// The command should be the first in the list of arguments
    void runCmd(const vector<string>& args)
    {
        ASSERT(args.size() > 0);

        const auto& s = *Nest_compilerSettings();

        Nest::Common::PrintTimer timer(s.verbose_, "", "   [%ws]\n");

        // Transform the strings into C-style strings, as required by ExecuteAndWait
        vector<const char*> cstrArgs;
        cstrArgs.reserve(args.size()+1);
        for ( const string& arg: args )
        {
            cstrArgs.push_back(arg.c_str());
            if ( s.verbose_ )
                cout << " " << arg;
        }
        cstrArgs.push_back(0);

        // Actually execute the program
        string errMsg;
        int res = llvm::sys::ExecuteAndWait(args[0], &cstrArgs[0], 0, 0, 0, 0, &errMsg);
        if ( res != 0 )
            REP_INTERNAL(NOLOC, "Cannot run command: %1%") % errMsg;
    }

    /// Write the given LLVM module, as a bitcode to disk
    void writeBitcodeFile(const Module& module, const string& outputFilename)
    {
        error_code errorInfo;
        unique_ptr<tool_output_file> outFile(new tool_output_file(outputFilename.c_str(), errorInfo, sys::fs::OpenFlags::F_None));
        if ( errorInfo )
            REP_INTERNAL(NOLOC, "Cannot generate bitcode file (%1%); reason: %2%") % outputFilename % errorInfo;

        llvm::WriteBitcodeToFile(&module, outFile->os());

        outFile->keep();
    }

    /// Write the given LLVM module, as an assembly file to disk
    void writeAssemblyFile(const Module& module, const string& outputFilename)
    {
        error_code errorInfo;
        unique_ptr<tool_output_file> outFile(new tool_output_file(outputFilename.c_str(), errorInfo, sys::fs::OpenFlags::F_None));
        if ( !outFile || errorInfo )
            REP_INTERNAL(NOLOC, "Cannot generate LLVM assembly file (%1%); reason: %2%") % outputFilename % errorInfo;

        outFile->os() << module;

        outFile->keep();
    }

    /// Generate optimized code from the given bitcode
    void generateOptimizedCode(const string& outputFilename, const string& inputFilename, const string& opt)
    {
        CompilerSettings& s = *Nest_compilerSettings();

        vector<string> args = { opt, "-std-link-opts", "-O" + s.optimizationLevel_ };
        args.insert(args.end(), s.optimizerArgs_.begin(), s.optimizerArgs_.end());
        args.insert(args.end(), { inputFilename, "-o", outputFilename });
        runCmd(args);

    }

    /// Generate machine native assembly from the given bitcode file
    void generateMachineAssembly(const string& outputFilename, const string& inputFilename, const string& llc)
    {
        CompilerSettings& s = *Nest_compilerSettings();

        vector<string> args = { llc, "--filetype=obj" };
#ifdef _WIN32
        //args.push_back("-mtriple");
        //args.push_back("i386-pc-mingw32");
        args.push_back("--march=x86-64");
#endif
        args.insert(args.end(), s.assemblerArgs_.begin(), s.assemblerArgs_.end());
        args.insert(args.end(), { inputFilename, "-o", outputFilename });
        runCmd(args);
    }

    // Given a bitcode file, generate a native object file
    void generateNativeObjGCC(const string& outputFilename, const string& inputFilename, const string& gcc)
    {
        CompilerSettings& s = *Nest_compilerSettings();

        // Run GCC to assemble and link the program into native code.
        //
        // Note:
        //  We can't just assemble and link the file with the system assembler
        //  and linker because we don't know where to put the _start symbol.
        //  GCC mysteriously knows how to do it.
        vector<string> args = { gcc, "-O0" };
        if ( s.generateDebugInfo_ )
            args.push_back("-g");

        for ( const string& str: s.libPaths_ )
        {
            args.push_back("-L" + str);
        }
        for ( const string& str: s.frameworkPaths_ )
        {
            args.push_back("-F" + str);
        }
        for ( const string& str: s.frameworks_ )
        {
            args.push_back("-framework");
            args.push_back(str);
        }
        args.insert(args.end(), s.linkerArgs_.begin(), s.linkerArgs_.end());
        args.insert(args.end(), { inputFilename, "-o", outputFilename });
        args.push_back("-lm");  // link against the math library
        runCmd(args);
    }
}



void LLVMB::generateRtAssembly(const llvm::Module& module)
{
    const auto& s = *Nest_compilerSettings();

    string filename = replaceExtension(s.output_, s.output_, ".ll");
    writeAssemblyFile(module, filename);
}

void LLVMB::generateCtAssembly(const llvm::Module& module)
{
    const auto& s = *Nest_compilerSettings();

    // Safety check
    if (!s.dumpCtAssembly_)
        return;

    namespace fs = boost::filesystem;

    // We will create one directory per compilation containing all the CT module dumps
    static int fileCounter = 0;
    static fs::path dirPath = replaceExtension(s.output_, s.output_, ".ct");
    if (fileCounter == 0) {
        // Remove the directory if exists
        fs::remove_all(dirPath);

        // Create the directory
        if (!fs::create_directory(dirPath))
            return;
    }

    // Get the actual file name (with path)
    int curFileIdx = fileCounter++;
    ostringstream filePath;
    filePath << dirPath.string() << "/ct_" << setfill('0') << setw(4) << curFileIdx << ".ll";
    writeAssemblyFile(module, filePath.str());
}

void LLVMB::link(const vector<llvm::Module*>& inputs, const string& outFilename)
{
    CompilerSettings& s = *Nest_compilerSettings();

    // TODO (modules): Fix this
    //
    // Some of the activities performed here should be performed per source code.
    // However, we don't have proper module linkage. Therefore, we combine all
    // the sourcecodes into one big module, and perform those operations here

    // Link all the input modules to a single module
    // we desotry all the modules in this process
    if ( inputs.empty() )
        REP_INTERNAL(NOLOC, "At least one bitcode needs to be passed to the linker");
    llvm::Module* compositeModule = inputs[0];
    llvm::Linker liner(*compositeModule);
    for ( size_t i=1; i<inputs.size(); ++i )
    {
        unique_ptr<llvm::Module> mod(inputs[i]);
        if ( liner.linkInModule(move(mod), llvm::Linker::OverrideFromSrc) )
            REP_INTERNAL(NOLOC, "Link error");
    }

    // Verify the module
    string err;
    raw_string_ostream errStream(err);
    if ( verifyModule(*compositeModule, &errStream) )
        REP_INTERNAL(NOLOC, "LLVM Verification failed for generated program: %1%") % err;

    bool shouldOptimize = !s.optimizerArgs_.empty() || s.optimizationLevel_ != "0";

    // Write the bitcode file -- this will typically be removed after generating the output files
    string bcFile = s.compileOnly_ && !shouldOptimize
                    ? outFilename
                    : replaceExtension(outFilename, outFilename, ".bc");
    llvm::FileRemover bcRemover(bcFile.c_str(), !s.keepIntermediateFiles_);
    writeBitcodeFile(*compositeModule, bcFile);

    string opt = s.executableDir_ + "/spr-opt";
    string llc = s.executableDir_ + "/spr-llc";
    string dis = s.executableDir_ + "/spr-llvm-dis";
#if defined(_WIN32) || defined(__CYGWIN__)
    opt += ".exe";
    llc += ".exe";
    dis += ".exe";
#endif


    // Optimize the module by calling the llvm optimizer, if we have some optimizer args
    if ( shouldOptimize )
    {
        string optFile = s.compileOnly_
                        ? outFilename
                        : replaceExtension(outFilename, outFilename, ".opt.bc");
        generateOptimizedCode(optFile, bcFile, opt);

        // Dump assembly if required
        if ( s.dumpOptAssembly_ )
        {
            // Disassembly the optimized file
            string optAsm = replaceExtension(outFilename, outFilename, ".opt.ll");
            runCmd({ dis, optFile, "-o", optAsm });
        }

        // Our source .bc file has changed; delete the prev file and continue from this one
        bcFile = optFile;
        bcRemover.setFile(bcFile, !s.keepIntermediateFiles_);
    }

    // Compile-only?
    if ( s.compileOnly_ )
    {
        bcRemover.releaseFile();
        return;
    }

    // Assemble (generate obj file)
    string objFile = s.compileAndAssembleOnly_
                    ? outFilename
                    : replaceExtension(outFilename, outFilename, ".o");
    FileRemover objFileRemover(objFile, !s.keepIntermediateFiles_);
    generateMachineAssembly(objFile, bcFile, llc);

    // Compile and assemble only?
    if ( s.compileAndAssembleOnly_ )
    {
        objFileRemover.releaseFile();
        return;
    }

    // TODO (modules):
    // Our link process has become just the transformation from obj file to executable.

    // Try to find GCC and call it to generate native code out of the assembly (or C)
    ErrorOr<string> gcc = sys::findProgramByName("gcc");
    if ( gcc.getError() )
        REP_INTERNAL(NOLOC, "Failed to find gcc");

    generateNativeObjGCC(outFilename, objFile.c_str(), gcc.get());
}
