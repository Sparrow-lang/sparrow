#include <StdInc.h>
#include "Generator.h"

#include <Nest/Common/Diagnostic.hpp>
#include <Nest/Common/PrintTimer.h>
#include <Nest/Compiler.h>
#include <Nest/CompilerSettings.h>

#include <boost/filesystem/path.hpp>

#include <algorithm>

#ifdef _MSC_VER
#pragma warning(push,1)
#endif
#include <llvm/Linker/Linker.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Bitcode/ReaderWriter.h>
#include <llvm/Support/FileUtilities.h>
#include <llvm/Support/Program.h>
#include <llvm/Support/ToolOutputFile.h>
#include <llvm/Support/raw_ostream.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

using namespace LLVMB;
using namespace llvm;
using namespace llvm::sys;

namespace
{
    /// Run the command with the given arguments
    /// The command should be the first in the list of arguments
    void runCmd(const vector<string>& args)
    {
        ASSERT(args.size() > 0);

        const auto& s = Nest::theCompiler().settings();

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
            REP_ERROR(NOLOC, "Cannot run command: %1%") % errMsg;
    }

    /// Write the given LLVM module, as a bitcode to disk
    void writeBitcodeFile(const Module& module, const string& outputFilename)
    {
        string errorInfo;
        unique_ptr<tool_output_file> outFile(new tool_output_file(outputFilename.c_str(), errorInfo, sys::fs::OpenFlags::F_None));
        if ( !errorInfo.empty() )
            REP_ERROR(NOLOC, "Cannot generate bitcode file (%1%); reason: %2%") % outputFilename % errorInfo;

        llvm::WriteBitcodeToFile(&module, outFile->os());

        outFile->keep();
    }

    /// Write the given LLVM module, as an assembly file to disk
    void writeAssemblyFile(const Module& module, const string& outputFilename)
    {
        string errorInfo;
        unique_ptr<tool_output_file> outFile(new tool_output_file(outputFilename.c_str(), errorInfo, sys::fs::OpenFlags::F_None));
        if ( !outFile || !errorInfo.empty() )
            REP_ERROR(NOLOC, "Cannot generate LLVM assembly file (%1%); reason: %2%") % outputFilename % errorInfo;

        outFile->os() << module;

        outFile->keep();
    }
    
    /// Generate optimized code from the given bitcode
    void generateOptimizedCode(const string& outputFilename, const string& inputFilename, const string& opt)
    {
        Nest::CompilerSettings& s = Nest::theCompiler().settings();

        vector<string> args = { opt, "-std-compile-opts", "-std-link-opts", "-O" + boost::lexical_cast<string>(s.optimizationLevel_) };
        args.insert(args.end(), s.optimizerArgs_.begin(), s.optimizerArgs_.end());
        args.insert(args.end(), { inputFilename, "-o", outputFilename });
        runCmd(args);

    }

    /// Generate machine native assembly from the given bitcode file
    void generateMachineAssembly(const string& outputFilename, const string& inputFilename, const string& llc)
    {
        Nest::CompilerSettings& s = Nest::theCompiler().settings();

        vector<string> args = { llc, "--filetype=obj" };
        if ( s.generateDebugInfo_ )
            args.emplace_back("-dwarf-version=3");
#ifdef _WIN32
        //args.push_back("-mtriple");
        //args.push_back("i386-pc-mingw32");
        args.push_back("--march=x86-64");
#endif
        args.insert(args.end(), s.generatorArgs_.begin(), s.generatorArgs_.end());
        args.insert(args.end(), { inputFilename, "-o", outputFilename });
        runCmd(args);
    }

    // Given a bitcode file, generate a native object file
    void generateNativeObjGCC(const string& outputFilename, const string& inputFilename, const string& gcc)
    {
        Nest::CompilerSettings& s = Nest::theCompiler().settings();

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
        runCmd(args);
    }
}



void LLVMB::generateAssembly(const llvm::Module& module, const string& outFilename)
{
    writeAssemblyFile(module, outFilename);
}

void LLVMB::link(const vector<llvm::Module*>& inputs, const string& outFilename)
{
    Nest::CompilerSettings& s = Nest::theCompiler().settings();

    // Link all the input modules to a single module
    // we desotry all the modules in this process
    if ( inputs.empty() )
        REP_INTERNAL(NOLOC, "At least one bitcode needs to be passed to the linker");
    llvm::Module* compositeModule = inputs[0];
    for ( size_t i=1; i<inputs.size(); ++i )
    {
        string errString;
        if ( !llvm::Linker::LinkModules(compositeModule, inputs[i], llvm::Linker::DestroySource, &errString) )
            REP_INTERNAL(NOLOC, "Link error: %1%") % errString;
    }

    // Verify the module
    string err;
    raw_string_ostream errStream(err);
    if ( verifyModule(*compositeModule, &errStream) )
        REP_ERROR(NOLOC, "LLVM Verification failed for generated program: %1%") % err;

    // Write the bitcode file -- this will typically be removed after generating the output files
    string bcFile = outFilename + ".bc";
    llvm::FileRemover bcRemover(bcFile.c_str(), !s.keepIntermediateFiles_);
    writeBitcodeFile(*compositeModule, bcFile);

    // If we have a simple linking, then the resuling bitcode is our output
    if ( s.simpleLinking_ )
    {
        bcRemover.releaseFile();
        return;
    }

    string opt = s.executableDir_ + "/llvm/opt";
    string llc = s.executableDir_ + "/llvm/llc";
    string dis = s.executableDir_ + "/llvm/llvm-dis";
#if defined(_WIN32) || defined(__CYGWIN__)
    opt += ".exe";
    llc += ".exe";
    dis += ".exe";
#endif


    // Optimize the module by calling the llvm optimizer, if we have some optimizer args
    if ( !s.optimizerArgs_.empty() || s.optimizationLevel_ > 0 )
    {
        string optFile = outFilename + ".opt.bc";
        generateOptimizedCode(optFile, bcFile, opt);

        // Dump assembly if required
        if ( s.dumpOptAssembly_ )
        {
            // Disassembly the optimized file
            runCmd({ dis, optFile, "-o", outFilename + ".opt.llvm" });
        }

        // Our source .bc file has changed; delete the prev file and continue from this one
        bcFile = optFile;
        bcRemover.setFile(bcFile, !s.keepIntermediateFiles_);
    }

    // If we are linking as a library, stop here
    if ( s.linkAsLibrary_ )
    {
        bcRemover.releaseFile();
        return;
    }

    // Generate assembly/obj file
    string objFile = outFilename + ".o";
    FileRemover objFileRemover(objFile, !s.keepIntermediateFiles_);
    generateMachineAssembly(objFile, bcFile, llc);

    // Try to find GCC and call it to generate native code out of the assembly (or C)
    string gcc = sys::FindProgramByName("gcc");
    if ( gcc.empty() )
        REP_ERROR(NOLOC, "Failed to find gcc");

    generateNativeObjGCC(outFilename, objFile.c_str(), gcc);
}
