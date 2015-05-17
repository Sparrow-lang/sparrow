#include <StdInc.h>
#include "Settings.h"

#include <Nest/Compiler.h>
#include <Nest/CompilerSettings.h>

#ifdef _MSC_VER
#pragma warning(disable: 4512)
#pragma warning(disable: 4239)
#endif

#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>

#include <fstream>

using namespace std;

namespace po = boost::program_options;

void initSettingsWithArgs(int argc, char** argv)
{
    Nest::CompilerSettings& s = Nest::theCompiler().settings();

    s.programName_ = argv[0];
    s.executableDir_ = boost::filesystem::system_complete(argv[0]).parent_path().string();
    
    // Declare a group of options that will be allowed only on command line
    po::options_description generic("Generic options");
    generic.add_options()
        ("version", "print version string")
        ("verbose,v", "Show commands to run and use verbose output")
        ("help,h", "produce help message")
        ;

    // Declare a group of options that will be allowed both on command line and in config file
    po::options_description config("Configuration");
    config.add_options()
        ("output,o", po::value<string>(&s.output_), "the executable output file name")
        ("import-path,i", po::value<vector<string> >(&s.importPaths_), "import path to search for Sparrow files")
        ("implicit-file,t", po::value<string>(&s.implicitLibFilePath_), "the path to the file containing implicit definitions")
        ("tab-size", po::value<int>(&s.tabSize_)->default_value(4), "number of spaces equivalent with a tab character")
        ("syntax-only", "check only the syntax")
        ("simple-linking,s", "link as a plain bitcode file, without optimization")
        ("link-as-library", "link as library")
        ("release,r", "enable release-mode compilation")
        ("debug-info,g", "generate debug info")
        ("fno-rvo", "disable RVO and pseudo-RVO")
        ("optimization,O", po::value<int>(&s.optimizationLevel_)->default_value(0), "the optimization level (0=disabled,1,2,3)")
        ("max-count-for-inline", po::value<int>(&s.maxCountForInline_)->default_value(30), "max number of lines for inlining")
        ("libraries-path,L", po::value<vector<string> >(&s.libPaths_), "import path to search for libraries")
        ("frameworks-path,F", po::value<vector<string> >(&s.frameworkPaths_), "import path to search for frameworks")
        ("libraries,l", po::value<vector<string> >(&s.libraries_), "library to link with the output")
        ("frameworks", po::value<vector<string> >(&s.frameworks_), "framework to link with the output")
        ("XOptimizer", po::value<vector<string> >(&s.optimizerArgs_), "argument to be passed to the optimizer")
        ("XGenerator", po::value<vector<string> >(&s.generatorArgs_), "argument to be passed to the generator")
        ("XLinker", po::value<vector<string> >(&s.linkerArgs_), "argument to be passed to the linker")
        ("dump-assembly", "dump LLVM assembly for the compilation units")
        ("dump-ct-assembly", "dump LLVM assembly for the CT module")
        ("dump-opt-assembly", "dump LLVM assembly for the optimized module")
        ("dump-ast", "dump AST for the compiled files")
        ("keep-intermediate-files", "keep intermediate files generating during compilation")
        ;

    // Hidden options, will be allowed both on command line and in config file, but will not be shown to the user.
    po::options_description hidden("Hidden options");
    hidden.add_options()
        ("input-file", po::value<vector<string> >(&s.filesToBeCompiled_), "input file")
        ;

    po::options_description cmdline_options;
    cmdline_options.add(generic).add(config).add(hidden);

    po::options_description config_file_options;
    config_file_options.add(config).add(hidden);

    po::options_description visible("Allowed options");
    visible.add(generic).add(config);


    po::positional_options_description p;
    p.add("input-file", -1);

    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(cmdline_options).positional(p).run(), vm);
    ifstream f("SparrowC.cfg");
    po::store(po::parse_config_file(f, config_file_options), vm);
    po::notify(vm);

    s.printVersion_ = vm.count("version") > 0;
    s.verbose_ = vm.count("verbose") > 0;
    s.syntaxOnly_ = vm.count("syntax-only") > 0;
    s.simpleLinking_ = vm.count("simple-linking") > 0;
    s.linkAsLibrary_ = vm.count("link-as-library") > 0;
    s.releaseMode_ = vm.count("release") > 0;
    s.generateDebugInfo_ = vm.count("debug-info") > 0;
    s.noRVO_ = vm.count("fno-rvo") > 0;
    s.dumpAssembly_ = vm.count("dump-assembly") > 0;
    s.dumpCtAssembly_ = vm.count("dump-ct-assembly") > 0;
    s.dumpOptAssembly_ = vm.count("dump-opt-assembly") > 0;
    s.dumpAST_ = vm.count("dump-ast") > 0;
    s.keepIntermediateFiles_ = vm.count("keep-intermediate-files") > 0;

#ifdef _WIN32
    s.keepIntermediateFiles_ = true;    // For some strange reason, we need this for Windows builds
#endif

    if ( vm.count("help") )
    {
        cout << visible << "\n";
    }
}
