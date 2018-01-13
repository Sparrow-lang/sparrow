#include <StdInc.h>
#include <Settings.h>
#include "AstDump.h"

#include "Nest/Api/Nest.h"
#include "Nest/Api/CompilerModule.h"
#include "Nest/Api/Compiler.h"
#include "Nest/Utils/CompilerSettings.hpp"
#include "Nest/Utils/Diagnostic.hpp"
#include "Nest/Utils/PrintTimer.hpp"
#include "Nest/Utils/StringRef.hpp"
#include "Nest/Api/Backend.h"
#include "Nest/Api/CompilationContext.h"
#include "Nest/Api/SourceCode.h"

#include "Feather/Api/Feather.h"
#include <LLVMBackend/LLVMBackendMod.h>
#include <SparrowFrontend/SparrowFrontend.h>

#include <boost/filesystem.hpp>
#include <boost/scoped_ptr.hpp>

#include <fstream>

using namespace Nest;

namespace fs = boost::filesystem;

extern SourceCode* g_implicitLibSC;
extern SourceCode* g_compilerArgsSC;


void _dumpAstForSourceCode(SourceCode* sourceCode, const char* fileSuffix) {
    auto& s = *Nest_compilerSettings();
    if ( s.dumpAST_.empty() || !strstr(sourceCode->url, s.dumpAST_.c_str()) )
        return;

    const char* end = strrchr(sourceCode->url, '.');
    if ( !end )
        end = sourceCode->url + strlen(sourceCode->url);
    string filename(sourceCode->url, end);
    filename += fileSuffix;
    filename += ".json";

    dumpAstNode(sourceCode->mainNode, filename.c_str());
}

void _onSourceCodeCreated(SourceCode* sourceCode)
{
    // Nothing to do for now
}
void _onSourceCodeParsed(SourceCode* sourceCode)
{
    _dumpAstForSourceCode(sourceCode, "_orig");
}
void _onSourceCodeCompiled(SourceCode* sourceCode)
{
    _dumpAstForSourceCode(sourceCode, "_comp");
}

void _onSourceCodeCodeGen(SourceCode* sourceCode)
{
    _dumpAstForSourceCode(sourceCode, "_gen");
}

bool tryImplicitLibPath(const char* relPath)
{
    auto& s = *Nest_compilerSettings();

    fs::path p(s.executableDir_ + relPath);
    if ( !exists(p) )
        return false;

    s.implicitLibFilePath_ = "SparrowImplicitLib.spr";
    s.importPaths_.push_back(p.string());
    return true;
}

bool ensureImplicitLib()
{
    // Only if we don't have an implicit lib set
    const auto& s = *Nest_compilerSettings();
    if ( s.implicitLibFilePath_ != "auto" )
        return true;

    return tryImplicitLibPath("/SparrowImplicitLib")
        || tryImplicitLibPath("/../SparrowImplicitLib")
        || tryImplicitLibPath("/../include/SparrowImplicitLib")
        || tryImplicitLibPath("/../../SparrowImplicitLib")
        || tryImplicitLibPath("/../../include/SparrowImplicitLib");
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
    if ( !f.is_open() ) {
        REP_INTERNAL(NOLOC, "Cannot create temporary file %1%") % tempPath;
        return;
    }

    // Add its content
    for ( const auto& def: defines ) {
        const char* suffix = "";
        if ( def.find('=') == string::npos ) {
            // If no value is given, assume it's 'true'
            suffix = " = true";
        }
        f << "using " << def << suffix << ";" << endl;
    }
    f.close();

    // Compile the generated file
    g_compilerArgsSC = Nest_compileFile(fromString(tempPath.native()));

    // Make sure we remove the file at the end
    fs::remove(tempPath);
}

void doCompilation(const vector<CompilerModule*>& modules)
{
    auto& s = *Nest_compilerSettings();

    ASSERT(!s.filesToBeCompiled_.empty());

    // Set the backend
    Nest_createBackend(s.filesToBeCompiled_[0].c_str());

    // Tell the modules we have a backend
    for ( CompilerModule* mod : modules )
    {
        if ( mod->onBackendSetFun )
            mod->onBackendSetFun(Nest_getCurBackend());
    }

    // Phase 1: Implicit lib
    {
        Nest::Common::PrintTimer timer(s.verbose_, s.implicitLibFilePath_.c_str(), "   [%ws]\n");

        // Process the implicit definitions file
        g_implicitLibSC = nullptr;
        if ( !s.implicitLibFilePath_.empty() )
            g_implicitLibSC = Nest_compileFile(fromString(s.implicitLibFilePath_));
    }

    // If we have some compiler defines, put them into a Sparrow source code
    if ( !s.defines_.empty() )
    {
        const char* filename = "argsDefines.spr";
        Nest::Common::PrintTimer timer(s.verbose_, filename, "   [%ws]\n");

        _handleArgDefines(s.defines_, filename);
    }


    // Parse each individual file
    for ( const auto& filename: s.filesToBeCompiled_  )
    {
        Nest::Common::PrintTimer timer(s.verbose_, "", "   [%ws]\n");
        if ( s.verbose_ )
            cout << filename;
        Nest_compileFile(fromString(filename));
    }

    // Also process the file that implements 'main' entry-point functionality
    if ( s.useMain_ ) {
        Nest::Common::PrintTimer timer(s.verbose_, "", "   [%ws]\n");
        if ( s.verbose_ )
            cout << "mainImpl.spr";
        Nest_compileFile(fromCStr("sprCore/mainImpl.spr"));
    }

    // If we have no errors, start linking
    if ( Nest_getErrorsNum() == 0 && !s.syntaxOnly_ )
    {
        try
        {
            Nest::Common::PrintTimer timer(s.verbose_, "", "[%ws]\n");
            if ( s.verbose_ )
                cout << "Linking..." << endl;
        	Nest_getCurBackend()->link(Nest_getCurBackend(), s.output_.c_str());
        }
        catch (const exception& e)
        {
            REP_INTERNAL(NOLOC, "Exception caught: '%1%'") % e.what();
        }
        catch (...)
        {
            REP_INTERNAL(NOLOC, "Unknown exception caught");
        }
    }
};

vector<CompilerModule*> gatherModules()
{
    vector<CompilerModule*> res;
    res.emplace_back(getNestModule());
    res.emplace_back(Feather_getModule());
    res.emplace_back(getLLVMBackendModule());
    res.emplace_back(getSparrowFrontendModule());
    return res;
}

int main(int argc,char* argv[])
{
    boost::timer::cpu_timer timer;
    timer.start();

    if ( !initSettingsWithArgs(argc, argv) )
        return 0;   // Successful exit

    const auto& s = *Nest_compilerSettings();

    // Make sure we have a valid path the Sparrow implicit lib
    if ( !ensureImplicitLib() )
    {
        cout << "Sparrow implicit lib not found" << endl;
        return 1;
    }

    // Initialize the modules
    vector<CompilerModule*> modules = gatherModules();
    for ( CompilerModule* mod : modules )
    {
        if ( mod->initFun )
            mod->initFun();
    }

    // Set our source code callbacks
    Nest_registerSourceCodeCreatedCallback(&_onSourceCodeCreated);
    Nest_registerSourceCodeParsedCallback(&_onSourceCodeParsed);
    Nest_registerSourceCodeCompiledCallback(&_onSourceCodeCompiled);
    Nest_registerSourceCodeCodeGenCallback(&_onSourceCodeCodeGen);

    try
    {
        doCompilation(modules);
    }
    catch (const exception& e)
    {
        REP_INTERNAL(NOLOC, "Exception caught: '%1%'") % e.what();
    }
    catch (...)
    {
        REP_INTERNAL(NOLOC, "Unknown exception caught");
    }

    // Destroy the modules
    for ( CompilerModule* mod : modules )
    {
        if ( mod->destroyFun )
            mod->destroyFun();
    }

    if ( s.verbose_ ) {
        timer.stop();
        cout << timer.format(3, "\nTime elapsed: %ws\n\n");
    }

    return 0;
}
