#include <StdInc.h>
#include <Settings.h>
#include "AstDump.h"

#include "Nest/Api/Nest.h"
#include "Nest/Api/CompilerModule.h"
#include "Nest/Api/Compiler.h"
#include "Nest/Utils/CompilerSettings.hpp"
#include "Nest/Utils/CompilerStats.hpp"
#include "Nest/Utils/Diagnostic.hpp"
#include "Nest/Utils/PrintTimer.hpp"
#include "Nest/Utils/Profiling.h"
#include "Nest/Utils/cppif/StringRef.hpp"
#include "Nest/Api/Backend.h"
#include "Nest/Api/CompilationContext.h"
#include "Nest/Api/SourceCode.h"

#include "Feather/Api/Feather.h"
#include <LLVMBackend/LLVMBackendMod.h>
#include <SparrowFrontend/SparrowFrontend.h>

#include <boost/filesystem.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/range/adaptor/reversed.hpp>

#include <fstream>

using namespace Nest;

namespace fs = boost::filesystem;

extern Nest_SourceCode* g_implicitLibSC;
extern Nest_SourceCode* g_compilerArgsSC;

void _dumpAstForSourceCode(Nest_SourceCode* sourceCode, const char* fileSuffix) {
    auto& s = *Nest_compilerSettings();
    if (s.dumpAST_.empty() || !strstr(sourceCode->url, s.dumpAST_.c_str()))
        return;

    const char* end = strrchr(sourceCode->url, '.');
    if (!end)
        end = sourceCode->url + strlen(sourceCode->url);
    string filename(sourceCode->url, end);
    filename += fileSuffix;
    filename += ".json";

    dumpAstNode(sourceCode->mainNode, filename.c_str());
}

void _onSourceCodeCreated(Nest_SourceCode* sourceCode) {
    // Nothing to do for now
}
void _onSourceCodeParsed(Nest_SourceCode* sourceCode) {
    _dumpAstForSourceCode(sourceCode, "_orig");
}
void _onSourceCodeCompiled(Nest_SourceCode* sourceCode) {
    _dumpAstForSourceCode(sourceCode, "_comp");
}

void _onSourceCodeCodeGen(Nest_SourceCode* sourceCode) {
    _dumpAstForSourceCode(sourceCode, "_gen");
}

bool tryImplicitLibPath(const char* relPath) {
    auto& s = *Nest_compilerSettings();

    fs::path p(s.executableDir_ + relPath);
    if (!exists(p))
        return false;

    s.implicitLibFilePath_ = "SparrowImplicitLib.spr";
    s.importPaths_.push_back(p.string());
    return true;
}

bool ensureImplicitLib() {
    // Only if we don't have an implicit lib set
    const auto& s = *Nest_compilerSettings();
    if (s.implicitLibFilePath_ != "auto")
        return true;

    return tryImplicitLibPath("/SparrowImplicitLib") ||
           tryImplicitLibPath("/../SparrowImplicitLib") ||
           tryImplicitLibPath("/../include/SparrowImplicitLib") ||
           tryImplicitLibPath("/../../SparrowImplicitLib") ||
           tryImplicitLibPath("/../../include/SparrowImplicitLib");
}

/// Handle compiler arg defines
///
/// We create a temporary file in which we put the arg defines as usings.
/// For the defines that don't have a value, we assign 'true'.
void _handleArgDefines(const vector<string>& defines, const char* filename) {

    // Create the temporary filename
    string tempFilename = fs::unique_path().native() + "-" + filename;
    fs::path tempPath = fs::temp_directory_path() / tempFilename;
    ofstream f(tempPath.native().c_str(), ofstream::out);
    if (!f.is_open()) {
        REP_INTERNAL(NOLOC, "Cannot create temporary file %1%") % tempPath;
        return;
    }

    // Add its content
    for (const auto& def : defines) {
        const char* suffix = "";
        if (def.find('=') == string::npos) {
            // If no value is given, assume it's 'true'
            suffix = " = true";
        }
        f << "using " << def << suffix << ";" << endl;
    }
    f.close();

    // Compile the generated file
    g_compilerArgsSC = Nest_compileFile(StringRef(tempPath.native()));

    // Make sure we remove the file at the end
    fs::remove(tempPath);
}

void doCompilation(const vector<Nest_CompilerModule*>& modules) {
    auto& s = *Nest_compilerSettings();

    ASSERT(!s.filesToBeCompiled_.empty());

    // Set the backend
    Nest_createBackend(s.filesToBeCompiled_[0].c_str());

    // Tell the modules we have a backend
    for (Nest_CompilerModule* mod : modules) {
        if (mod->onBackendSetFun)
            mod->onBackendSetFun(Nest_getCurBackend());
    }

    // Phase 1: Implicit lib
    {
        Nest::Common::PrintTimer timer(s.verbose_, s.implicitLibFilePath_.c_str(), "   [%d ms]\n");

        // Gather statistics if requested
        CompilerStats& stats = CompilerStats::instance();
        ScopedTimeCapture timeCapture(stats.enabled, stats.timeImplicitLib);

        // Process the implicit definitions file
        g_implicitLibSC = nullptr;
        if (!s.implicitLibFilePath_.empty())
            g_implicitLibSC = Nest_compileFile(StringRef(s.implicitLibFilePath_));
    }

    // If we have some compiler defines, put them into a Sparrow source code
    if (!s.defines_.empty()) {
        const char* filename = "argsDefines.spr";
        Nest::Common::PrintTimer timer(s.verbose_, filename, "   [%d ms]\n");

        _handleArgDefines(s.defines_, filename);
    }

    // Parse each individual file
    for (const auto& filename : s.filesToBeCompiled_) {
        Nest::Common::PrintTimer timer(s.verbose_, "", "   [%d ms]\n");
        if (s.verbose_)
            cout << filename;
        Nest_compileFile(StringRef(filename));
    }

    // Also process the file that implements 'main' entry-point functionality
    if (s.useMain_) {
        Nest::Common::PrintTimer timer(s.verbose_, "", "   [%d ms]\n");
        if (s.verbose_)
            cout << "mainImpl.spr";
        Nest_compileFile(StringRef("sprCore/mainImpl.spr"));
    }

    // If we have no errors, start linking
    if (Nest_getErrorsNum() == 0 && !s.syntaxOnly_) {
        PROFILING_ZONE_NAMED("Linking")
        try {
            Nest::Common::PrintTimer timer(s.verbose_, "", "[%d ms]\n");
            if (s.verbose_)
                cout << "Linking..." << endl;
            Nest_getCurBackend()->link(Nest_getCurBackend(), s.output_.c_str());
        } catch (const exception& e) {
            REP_INTERNAL(NOLOC, "Exception caught: '%1%'") % e.what();
        } catch (...) {
            REP_INTERNAL(NOLOC, "Unknown exception caught");
        }
    }
};

vector<Nest_CompilerModule*> gatherModules() {
    vector<Nest_CompilerModule*> res;
    res.emplace_back(getNestModule());
    res.emplace_back(Feather_getModule());
    res.emplace_back(getLLVMBackendModule());
    res.emplace_back(getSparrowFrontendModule());
    return res;
}

// NOLINTNEXTLINE(bugprone-exception-escape)
int main(int argc, char* argv[]) {
    CompilerStats& stats = CompilerStats::instance();
    auto startTime = chrono::steady_clock::now();

    if (!initSettingsWithArgs(argc, argv))
        return 0; // Successful exit

    const auto& s = *Nest_compilerSettings();

    // Make sure we have a valid path the Sparrow implicit lib
    if (!ensureImplicitLib()) {
        cout << "Sparrow implicit lib not found" << endl;
        return 1;
    }

    // Initialize the modules
    vector<Nest_CompilerModule*> modules = gatherModules();
    for (Nest_CompilerModule* mod : modules) {
        if (mod->initFun)
            mod->initFun();
    }

    // Set our source code callbacks
    Nest_registerSourceCodeCreatedCallback(&_onSourceCodeCreated);
    Nest_registerSourceCodeParsedCallback(&_onSourceCodeParsed);
    Nest_registerSourceCodeCompiledCallback(&_onSourceCodeCompiled);
    Nest_registerSourceCodeCodeGenCallback(&_onSourceCodeCodeGen);

    try {
        doCompilation(modules);
    } catch (const exception& e) {
        REP_INTERNAL(NOLOC, "Exception caught: '%1%'") % e.what();
    } catch (...) {
        REP_INTERNAL(NOLOC, "Unknown exception caught");
    }

    // Destroy the modules (in reverse order of initialization)
    for (Nest_CompilerModule* mod : boost::adaptors::reverse(modules)) {
        if (mod->destroyFun)
            mod->destroyFun();
    }

    auto endTime = chrono::steady_clock::now();
    auto durMs = chrono::duration_cast<chrono::milliseconds>(endTime - startTime);
    if (s.verbose_) {
        printf("\nTime elapsed: %.3f s\n\n", durMs.count() / 1000.0);
    }
    if (stats.enabled) {
        printf("\nCompiler stats:\n");
        printf("#TotalTime: %d\n", (int)durMs.count());
        printf("#ImplicitLibTime: %d\n", (int)stats.timeImplicitLib.count() / 1000);
        printf("#LlcTime: %d\n", (int)stats.timeLlc.count() / 1000);
        printf("#OptTime: %d\n", (int)stats.timeOpt.count() / 1000);
        printf("#FinalLinkTime: %d\n", (int)stats.timeLink.count() / 1000);
        printf("#NumCtEvals: %d\n", stats.numCtEvals);
        printf("#CtEvalsTime: %d\n", (int)stats.timeCtEvals.count() / 1000);
        printf("\n");
    }

    return 0;
}
