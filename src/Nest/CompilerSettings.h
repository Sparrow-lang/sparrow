#pragma once

#include <string>
#include <vector>

namespace Nest
{
    /// Class that holds the compiler settings
    class CompilerSettings
    {
    public:
        CompilerSettings()
            : printVersion_(false)
            , verbose_(false)
            , implicitLibFilePath_("auto")
            , tabSize_(4)
            , syntaxOnly_(false)
            , simpleLinking_(false)
            , linkAsLibrary_(false)
            , releaseMode_(false)
            , generateDebugInfo_(false)
            , optimizationLevel_(0)
            , noRVO_(false)
            , maxCountForInline_(30)
            , dataLayout_("e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64")
            , targetTriple_("x86_64-apple-macosx10.9.0")
            , dumpAssembly_(false)
            , dumpCtAssembly_(false)
            , dumpOptAssembly_(false)
            , dumpAST_(false)
            , keepIntermediateFiles_(false)
        {}

    /// Deduced settings
    ///{
        /// The name of the program
        string programName_;

        /// The directory where the executable is
        string executableDir_;
    ///}


    /// General compiler settings
    ///{
        /// If true, the compiler should print the current version and exit
        bool printVersion_;

        /// Show commands to run and use verbose output
        bool verbose_;

        /// The output file name
        string output_;

        /// The files that need to be compiled
        vector<string> filesToBeCompiled_;

        /// Import paths for Sparrow files
        vector<string> importPaths_;

        /// The location of the file that needs to be loaded as implicit lib (containing standard types)
        string implicitLibFilePath_;
    ///}

    /// Frontend specific settings
    ///{
        // The size of a tab character in spaces
        int tabSize_;
    ///}

    /// Backend specific settings
    ///{
        /// If true compile only syntax
        bool syntaxOnly_;

        /// If this is true, we will produce a simple .bc file as output, without optimization
        bool simpleLinking_;

        /// True if we should link as a library, instead of an executable
        bool linkAsLibrary_;

        /// True if we are building a release configuration for our program
        bool releaseMode_;

        /// True if we should generate debug information
        bool generateDebugInfo_;

        /// The optimization level to be applied to the generated code (0=no optimizations,1,2,3)
        int optimizationLevel_;

        /// If this is set, we disable ReturnValueOptimization (and pseudo-RVO)
        bool noRVO_;

        /// The maximum number of lines to be considered for inline; after this number of lines, the functions will not
        /// be inlined
        int maxCountForInline_;

        /// The LLVM data layout that we are using
        string dataLayout_;

        /// The target triple to be used
        string targetTriple_;

        /// List of paths where we can find libraries that we are linking with
        vector<string> libPaths_;

        /// List of paths where we can find frameworks that we are linking with
        vector<string> frameworkPaths_;

        /// List of libraries to link with
        vector<string> libraries_;

        /// List of frameworks to link with
        vector<string> frameworks_;

        /// List of arguments to be passed to the optimizer
        vector<string> optimizerArgs_;

        /// List of arguments to be passed to the code generator
        vector<string> generatorArgs_;

        /// List of arguments to be passed to the linker
        vector<string> linkerArgs_;
    ///}

    /// Debugging
    ///{
        /// If true we will dump LLVM assembly for each input file, and for the output file
        bool dumpAssembly_;

        /// If true we will dump LLVM assembly for the CT module
        bool dumpCtAssembly_;

        /// If true we will dump LLVM assembly for the optimized module
        bool dumpOptAssembly_;

        /// If true we will dump the AST corresponding to the given source code, and to the compiled code
        bool dumpAST_;

        /// Set this to true if you don't want to delete the intermediate files used during generation
        bool keepIntermediateFiles_;
    ///}
    };
}
