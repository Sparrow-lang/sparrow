#pragma once

#include <string>
#include <vector>

/// Class that holds the compiler settings
struct Nest_CompilerSettings {
    /// Deduced settings
    ///{
    /// The name of the program
    string programName_;

    /// The directory where the executable is
    string executableDir_;
    ///}

    /// Generic options
    ///{
    /// If true, the compiler should print the current version and exit
    bool printVersion_{false};

    /// Show commands to run and use verbose output
    bool verbose_{false};

    /// True if we shouldn't use colors when displaying error messages
    bool noColors_{false};
    ///}

    /// Driver options
    ///{

    /// The files that need to be compiled
    vector<string> filesToBeCompiled_;

    /// The output file name
    string output_;

    /// If true compile only syntax, including type checking
    bool syntaxOnly_{false};

    /// Compile & assemble only; no linking
    bool compileAndAssembleOnly_{false};

    /// Compile only
    bool compileOnly_{false};

    /// List of arguments to be passed to the assembler
    vector<string> assemblerArgs_;

    /// List of arguments to be passed to the optimizer
    vector<string> optimizerArgs_;

    /// List of arguments to be passed to the linker
    vector<string> linkerArgs_;

    ///}

    /// Directory options
    ///{
    /// Import paths for Sparrow files
    vector<string> importPaths_;

    /// List of paths where we can find libraries that we are linking with
    vector<string> libPaths_;

    /// List of paths where we can find frameworks that we are linking with
    vector<string> frameworkPaths_;
    ///}

    /// Sparrow specific options
    ///{
    /// The location of the file that needs to be loaded as implicit lib (containing standard types)
    string implicitLibFilePath_{"auto"};

    /// True if we need to include the entry point functionality into the final program
    bool useMain_{true};
    ///}

    /// Code generation options
    ///{
    /// The compiler defines passed as arguments
    vector<string> defines_;

    /// True if we should generate debug information
    bool generateDebugInfo_{false};

    /// The LLVM data layout that we are using
    string dataLayout_;

    /// The target triple to be used
    string targetTriple_;

    /// The maximum number of lines to be considered for inline; after this number of lines, the
    /// functions will not be inlined
    int maxCountForInline_{30};

    /// The optimization level to be applied to the generated code (0=no optimizations,1,2,3, 's',
    /// 'fast')
    string optimizationLevel_{"0"};

    /// If this is set, we disable ReturnValueOptimization (and pseudo-RVO)
    bool noRVO_{false};
    ///}

    /// Linker options
    ///{
    /// List of libraries to link with
    vector<string> libraries_;

    /// List of frameworks to link with
    vector<string> frameworks_;
    ///}

    /// Debugging options
    ///{
    /// If true we will dump LLVM assembly for each input file, and for the output file
    bool dumpAssembly_{false};

    /// If true we will dump LLVM assembly for the CT module
    bool dumpCtAssembly_{false};

    /// If true we will dump LLVM assembly for the optimized module
    bool dumpOptAssembly_{false};

    /// If given, the AST will be dumped for original source code and compiled code
    /// This specifies a filter to be applied to the list of source codes that the compiler
    /// will compile
    string dumpAST_;

    /// Set this to true if you don't want to delete the intermediate files used during generation
    bool keepIntermediateFiles_{false};
    ///}
};
