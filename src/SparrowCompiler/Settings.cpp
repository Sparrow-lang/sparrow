#include <StdInc.h>
#include "Settings.h"

#include "Nest/Api/Compiler.h"
#include "Nest/Utils/CompilerSettings.hpp"

#ifdef _MSC_VER
#pragma warning(disable: 4512)
#pragma warning(disable: 4239)
#endif

#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>

#include <fstream>
#include <unordered_set>
#include <unordered_map>

using namespace std;

namespace po = boost::program_options;

static const int bufferSize = 1024;

string getExecutablePathFallback(const char* argv0)
{
    if ( argv0 == nullptr || argv0[0] == 0 )
        return "";

    boost::system::error_code ec;
    boost::filesystem::path p(
        boost::filesystem::canonical(
            argv0, boost::filesystem::current_path(), ec));
    return p.make_preferred().string();
}

#if (BOOST_OS_CYGWIN || BOOST_OS_WINDOWS)

    #include <Windows.h>

    string getExecutablePath(const char* argv0)
    {
        char buf[bufferSize] = {0};
        DWORD ret = GetModuleFileNameA(NULL, buf, sizeof(buf));
        if ( ret == 0 || ret == sizeof(buf) )
            return getExecutablePathFallback(argv0);
        return buf;
    }

#elif (BOOST_OS_MACOS)

    #include <mach-o/dyld.h>

    string getExecutablePath(const char* argv0)
    {
        char buf[bufferSize] = {0};
        uint32_t size = sizeof(buf);
        int ret = _NSGetExecutablePath(buf, &size);
        if ( 0 != ret )
            return getExecutablePathFallback(argv0);

        boost::system::error_code ec;
        boost::filesystem::path p(
            boost::filesystem::canonical(buf, boost::filesystem::current_path(), ec));
        return p.make_preferred().string();
    }

#elif (BOOST_OS_SOLARIS)

    #include <stdlib.h>

    string getExecutablePath(const char* argv0)
    {
        string ret = getexecname();
        if ( ret.empty() )
            return getExecutablePathFallback(argv0);

        boost::filesystem::path p(ret);
        if ( !p.has_root_directory() )
        {
            boost::system::error_code ec;
            p = boost::filesystem::canonical(
                p, boost::filesystem::current_path(), ec);
            ret = p.make_preferred().string();
        }
        return ret;
    }

#elif (BOOST_OS_BSD)

    #include <sys/sysctl.h>

    string getExecutablePath(const char* argv0)
    {
        int mib[4] = {0};
        mib[0] = CTL_KERN;
        mib[1] = KERN_PROC;
        mib[2] = KERN_PROC_PATHNAME;
        mib[3] = -1;
        char buf[bufferSize] = {0};
        size_t size = sizeof(buf);
        sysctl(mib, 4, buf, &size, NULL, 0);
        if ( size == 0 || size == sizeof(buf) )
            return getExecutablePathFallback(argv0);

        string path(buf, size);
        boost::system::error_code ec;
        boost::filesystem::path p(
            boost::filesystem::canonical(
                path, boost::filesystem::current_path(), ec));
        return p.make_preferred().string();
    }

#elif (BOOST_OS_LINUX)

    #include <unistd.h>

    string getExecutablePath(const char* argv0)
    {
        char buf[bufferSize] = {0};
        ssize_t size = readlink("/proc/self/exe", buf, sizeof(buf));
        if ( size == 0 || size == sizeof(buf) )
            return getExecutablePathFallback(argv0);

        string path(buf, size);
        boost::system::error_code ec;
        boost::filesystem::path p(
            boost::filesystem::canonical(
                path, boost::filesystem::current_path(), ec));
        return p.make_preferred().string();
    }

#else

string getExecutablePath(const char* argv0)
{
    return getExecutablePathFallback(argv0);
}

#endif // }

/// Class that knows how to handle an option
class OptionHandler {
    /// Enum defining the possible action types
    enum ActionType {
        doNothing = 0,  //!< Do nothing
        storeTrue,      //!< Store true in the given bool variable (no arg expected)
        storeFalse,     //!< Store false in the given bool variable (no arg expected)
        storeInt,       //!< Store the given arg in the given int variable
        storeString,    //!< Store the given arg in the given string variable
        appendString,   //!< Add the given arg to the given list of string
    };

    /// The type of action to perform when parsing the option
    ActionType actionType_;

    /// The value given to the constructor that should be changed
    union {
        bool* boolVal;
        int* intVal;
        string* stringVal;
        vector<string>* stringListVal;
    } val_;

public:
    OptionHandler() : actionType_(doNothing) {
        val_.boolVal = nullptr;
    }

    OptionHandler(bool& res, bool store = true) : actionType_(store ? storeTrue : storeFalse) {
        val_.boolVal = &res;
    }

    OptionHandler(int& res) : actionType_(storeInt) {
        val_.intVal = &res;
    }

    OptionHandler(string& res) : actionType_(storeString) {
        val_.stringVal = &res;
    }

    OptionHandler(vector<string>& res) : actionType_(appendString) {
        val_.stringListVal = &res;
    }

    /// Called when an option is encountered, passing the value of the option
    void onOption(const char* arg) {
        switch ( actionType_ ) {
        case storeTrue:
            *val_.boolVal = true;
            break;
        case storeFalse:
            *val_.boolVal = false;
            break;
        case storeInt:
            *val_.intVal = boost::lexical_cast<int>(arg);
            break;
        case storeString:
            *val_.stringVal = arg;
            break;
        case appendString:
            val_.stringListVal->emplace_back(arg);
            break;
        case doNothing:
        default:
            break;
        }
    }
};


/// Structure that describes an entry in the options list
struct OptionEntry {
    const char* tag;
    const char* argName;    // NULL=no arg, starts with space => next arg, otherwise concat
    OptionHandler handler;
    const char* helpMessage;
};

/// Structure describing the parsed arguments
struct ParsedArgs {
    /// All the positional parameters, that are not options
    std::vector<string> positionalParams;
    /// All the flags we have (without arguments)
    unordered_set<string> flags;
    /// All the options with all their arguments
    unordered_map<string, std::vector<string> > optionsWithArgs;
};

/// Tests if the given flag is set in the parsed arguments
bool hasFlag(const ParsedArgs& args, const char* flag) {
    return args.flags.end() != args.flags.find(flag);
}

/// Try to retrieve the value of the option with the given tag.
/// If the option is not specified, the resulting variable isn't changed
void tryRetrieveOptionValue(const ParsedArgs& args, const char* optionTag, string& res) {
    auto it = args.optionsWithArgs.find(optionTag);
    if ( it != args.optionsWithArgs.end() ) {
        const auto& values = it->second;
        if ( !values.empty() )
            res = values.back();    // Used the last value given
    }
}

void tryRetrieveOptionValue(const ParsedArgs& args, const char* optionTag, int& res) {
    auto it = args.optionsWithArgs.find(optionTag);
    if ( it != args.optionsWithArgs.end() ) {
        const auto& values = it->second;
        if ( !values.empty() ) {
            res = boost::lexical_cast<int>(values.back());
        }
    }
}

/// Gets all the values passed as option arguments for the given tag
vector<string> getArgValues(const ParsedArgs& args, const char* optionTag) {
    auto it = args.optionsWithArgs.find(optionTag);
    if ( it != args.optionsWithArgs.end())
        return it->second;
    else
        return {};
}

/// Parse the arguments according to the given option entries
ParsedArgs parseArgs(int argc, char** argv, OptionEntry* optionsMap, int numOptions, bool& hasErrors) {
    ParsedArgs res;

    for ( int i=1; i<argc; i++ ) {
        const char* arg = argv[i];

        int len = strlen(arg);
        if ( len > 0 && arg[0] == '-' ) {
            // Option arg
            bool handled = false;

            // Go over all our option entries, see if anyone matches
            for ( int k=0; k<numOptions; k++ ) {
                OptionEntry& entry = optionsMap[k];
                if ( !entry.tag ) continue;

                if ( entry.argName && entry.argName[0] != ' ' ) {
                    // Option argument is concatenated
                    int tagLen = strlen(entry.tag);
                    if ( tagLen < len ) {
                        const char* p = strstr(arg, entry.tag);
                        if ( p == arg ) {
                            // We have a match
                            p += tagLen;    // The actual option argument
                            res.optionsWithArgs[entry.tag].emplace_back(p);
                            entry.handler.onOption(p);
                            handled = true;
                            break;
                        }
                    }
                }
                else {
                    // We need a full match
                    if ( 0 == strcmp(arg, entry.tag) ) {
                        if ( entry.argName ) {
                            // Next arg is the argument of this option
                            if ( i+1 == argc ) {
                                cout << "Missing argument for option '" << arg << "'" << endl;
                                hasErrors = true;
                                handled = true;
                                break;
                            }
                            const char* optArg = argv[++i];
                            res.optionsWithArgs[entry.tag].emplace_back(optArg);
                            entry.handler.onOption(optArg);
                            handled = true;
                            break;
                        }
                        else {
                            // No option argument
                            res.flags.insert(entry.tag);
                            entry.handler.onOption("");
                            handled = true;
                            break;
                        }
                    }
                }
            }
            if ( !handled ) {
                cout << "Invalid argument '" << arg << "'" << endl;
                hasErrors = true;
            }
        }
        else {
            // Positional arg
            res.positionalParams.emplace_back(arg);
        }
    }

    return res;
}

/// Print help message according with the given option entries
void printHelp(OptionEntry* optionsMap, int numOptions) {
    CompilerSettings& s = *Nest_compilerSettings();
    cout << "USAGE: " << s.programName_ << " [options] <inputs>" << endl;

    static const int leftSize = 30;
    int written = 0;
    for ( int i=0; i<numOptions; ++i ) {
        OptionEntry& entry = optionsMap[i];

        if ( !entry.tag ) {
            cout << endl;
            cout << entry.helpMessage << ":" << endl;
            written = 0;
        }
        else
        {
            if ( written > 0 ) {
                cout << ' ';
                written++;
            }
            if ( entry.tag ) {
                cout << entry.tag;
                written += strlen(entry.tag);
            }
            if ( entry.argName ) {
                cout << entry.argName;
                written += strlen(entry.argName);
            }
            if ( entry.helpMessage ) {
                // Align the help message
                if ( written >= leftSize ) {
                    cout << endl;
                    written = 0;
                }
                for ( int i=0; i<leftSize-written; ++i )
                    cout << ' ';

                cout << entry.helpMessage << endl;
                written = 0;
            }
        }
    }
}

bool initSettingsWithArgs(int argc, char** argv)
{
    CompilerSettings& s = *Nest_compilerSettings();

    s.programName_ = argv[0];
    s.executableDir_ = boost::filesystem::system_complete(getExecutablePath(argv[0])).parent_path().string();

    // Default data layout & target triple
    s.dataLayout_ = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64";
#if (BOOST_OS_MACOS)
    s.targetTriple_ = "x86_64-apple-macosx10.9.0";
#else
    s.targetTriple_ = "x86_64-linux-gnu";
#endif

    bool showHelp = false;

    OptionEntry optionsMap[] = {
        { NULL,                     NULL, {}, "Generic options" },
        { "-h",                     NULL, showHelp, NULL },
        { "-help",                  NULL, showHelp, "produce this help message" },
        { "-version",               NULL, s.printVersion_, "just print the version string" },
        { "-v",                     NULL, s.verbose_, "show commands to run and use verbose output" },
        { "-fno-colors",            NULL, s.noColors_, "disable colors in the output" },

        { NULL,                     NULL, {}, "Driver options" },
        { "-o",                     " <file>", s.output_, "the executable output file name" },
        { "-fsyntax-only",          NULL, s.syntaxOnly_, "check only the syntax, including type checking" },
        { "-c",                     NULL, s.compileAndAssembleOnly_, "compile and assemble, don't link; generates object files" },
        { "-S",                     NULL, s.compileOnly_, "compile only; generates assembly file (.bc)" },
        { "-Xassembler",            " <arg>", s.assemblerArgs_, "argument to be passed to the assembler" },
        { "-XOptimizer",            " <arg>", s.optimizerArgs_, "argument to be passed to the optimizer" },
        { "-XLinker",               " <arg>", s.linkerArgs_, "argument to be passed to the linker" },


        { NULL,                     NULL, {}, "Directory options" },
        { "-I",                     "<dir>", s.importPaths_, "import path to search for Sparrow files" },
        { "-L",                     "<dir>", s.libPaths_, "import path to search for libraries" },
        { "-F",                     "<dir>", s.frameworkPaths_, "import path to search for frameworks" },

        { NULL,                     NULL, {}, "Sparrow specific options" },
        { "-t",                     " <file>", s.implicitLibFilePath_, NULL },
        { "-implicit-file",         " <file>", s.implicitLibFilePath_, "the path to the file containing implicit definitions" },
        { "-fmain",                 NULL, {s.useMain_, true}, "include entry point functionality" },
        { "-fno-main",              NULL, {s.useMain_, false}, "don't include entry point functionality" },

        { NULL,                     NULL, {}, "Code generation options" },
        { "-g",                     NULL, s.generateDebugInfo_, "generate debug info" },
        { "-data-layout",           " <arg>", s.dataLayout_, "the data layout" },
        { "-target-triple",         " <arg>", s.targetTriple_, "the target machine triple" },
        { "-max-count-for-inline",  " <n>", s.maxCountForInline_, "max number of lines for inlining" },
        { "-O0",                    NULL, {}, "no optimizations (default)" },
        { "-O1",                    NULL, {}, "light optimizations" },
        { "-O2",                    NULL, {}, "enable most optimizations" },
        { "-O3",                    NULL, {}, "also enable optimizations that take longer or generate larger code" },
        { "-Os",                    NULL, {}, "optimize for code size" },
        { "-Ofast",                 NULL, {}, "optimize for fast code; enable aggresive optimizations" },
        { "-fno-rvo",               NULL, s.noRVO_, "disable RVO and pseudo-RVO" },

        { NULL,                     NULL, {}, "Linker options" },
        { "-l",                     " <file>", s.libraries_, "library to link with the output" },
        { "-framework",             " <file>", s.frameworks_, "framework to link with the output" },

        { NULL,                     NULL, {}, "Debugging options" },
        { "-dump-assembly",         NULL, s.dumpAssembly_, "dump LLVM assembly for the compilation units" },
        { "-dump-ct-assembly",      NULL, s.dumpCtAssembly_, "dump LLVM assembly for the CT module" },
        { "-dump-opt-assembly",     NULL, s.dumpOptAssembly_, "dump LLVM assembly for the optimized module" },
        { "-dump-ast",              " <filter>", s.dumpAST_, "dump AST for the files matching the given filter" },
        { "-keep-intermediate-files", NULL, s.keepIntermediateFiles_, "keep intermediate files generating during compilation" },
    };
    static const int numOptions = sizeof(optionsMap)/sizeof(optionsMap[0]);

    bool hasErrors = false;
    ParsedArgs args = parseArgs(argc, argv, optionsMap, numOptions, hasErrors);

    s.filesToBeCompiled_ = args.positionalParams;

#ifdef _WIN32
    s.keepIntermediateFiles_ = true;    // For some strange reason, we need this for Windows builds
#endif

    if ( hasFlag(args, "-O1") )
        s.optimizationLevel_ = "1";
    if ( hasFlag(args, "-O2") )
        s.optimizationLevel_ = "2";
    if ( hasFlag(args, "-O3") )
        s.optimizationLevel_ = "3";
    if ( hasFlag(args, "-Os") )
        s.optimizationLevel_ = "s";
    if ( hasFlag(args, "-Ofast") )
        s.optimizationLevel_ = "fast";

    // Compute the output filename, if one is not given
    if ( s.output_.empty() ) {
        string extension = ".out";
    #if defined(_WIN32) || defined(__CYGWIN__)
        extension = ".exe";
    #endif

        if ( s.compileAndAssembleOnly_ )
            extension = ".o";
        else  if ( s.compileOnly_ )
            extension = ".bc";

        if ( !s.filesToBeCompiled_.empty() )
            s.output_ = boost::filesystem::path(s.filesToBeCompiled_[0]).replace_extension(extension).string();
        else
            s.output_ = "a" + extension;
    }

    if ( hasFlag(args, "-h") || hasFlag(args, "-help") )
    {
        printHelp(optionsMap, numOptions);
        cout << endl;
        return false;
    }

    if ( s.filesToBeCompiled_.empty() ) {
        cout << "No input file was given!" << endl;
        hasErrors = true;
    }

    if ( hasErrors ) {
        cout << endl;
        cout << "Run '" << s.programName_ << " -help' for the list of available options" << endl;
        cout << endl;
        return false;
    }

    return true;
}
