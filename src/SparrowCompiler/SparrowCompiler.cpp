#include <StdInc.h>
#include <Settings.h>

#include "Nest/Api/Nest.h"
#include "Nest/Api/CompilerModule.h"
#include "Nest/Api/Compiler.h"
#include "Nest/Utils/CompilerSettings.hpp"
#include "Nest/Utils/Diagnostic.hpp"
#include "Nest/Utils/PrintTimer.hpp"
#include "Nest/Utils/StringRef.hpp"
#include "Nest/Api/Backend.h"
#include "Nest/Api/CompilationContext.h"

#include "Feather/Api/Feather.h"
#include <LLVMBackend/LLVMBackendMod.h>
#include <SparrowFrontend/SparrowFrontend.h>

#include <boost/filesystem.hpp>
#include <boost/scoped_ptr.hpp>

using namespace Nest;

namespace fs = boost::filesystem;

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
        || tryImplicitLibPath("/../../SparrowImplicitLib");
}

void doCompilation(const vector<CompilerModule*>& modules)
{
    const auto& s = *Nest_compilerSettings();

    ASSERT(!s.filesToBeCompiled_.empty());

    // Set the backend
    Nest_createBackend(s.filesToBeCompiled_[0].c_str());

    // Tell the modules we have a backend
    for ( CompilerModule* mod : modules )
    {
        if ( mod->onBackendSetFun )
            mod->onBackendSetFun(Nest_getCurBackend());
    }

    // Compute the output filename
    string extension = ".out";
#if defined(_WIN32) || defined(__CYGWIN__)
    extension = ".exe";
#endif
    string outFilename;
    if ( !s.filesToBeCompiled_.empty() )
        outFilename = boost::filesystem::path(s.filesToBeCompiled_[0]).replace_extension(extension).string();
    else
        outFilename = "a" + extension;

    // Phase 1: Implicit lib
    {
        Nest::Common::PrintTimer timer(s.verbose_, "<implicit lib>", "   [%ws]\n");

        // Process the implicit definitions file
        if ( !s.implicitLibFilePath_.empty() )
            Nest_compileFile(fromString(s.implicitLibFilePath_));
    }


    // Parse each individual file
    for ( const auto& filename: s.filesToBeCompiled_  )
    {
        Nest::Common::PrintTimer timer(s.verbose_, "", "   [%ws]\n");
        cout << filename;
        Nest_compileFile(fromString(filename));
        if ( !s.verbose_ )
            cout << endl;
    }

    // If we have no errors, start linking
    if ( Nest_getErrorsNum() == 0 && !s.syntaxOnly_ )
    {
        try
        {
            Nest::Common::PrintTimer timer(s.verbose_, "", "[%ws]\n");
            cout << "Linking..." << endl;
        	Nest_getCurBackend()->link(Nest_getCurBackend(), outFilename.c_str());
        }
        catch (...)
        {
        }
    }
};

vector<CompilerModule*> gatherModules()
{
    vector<CompilerModule*> res;
    res.emplace_back(getNestModule());
    res.emplace_back(getFeatherModule());
    res.emplace_back(getLLVMBackendModule());
    res.emplace_back(getSparrowFrontendModule());
    return res;
}

int main(int argc,char* argv[])
{
    cout << "Sparrow Compiler v0.9.3, (c) 2015 Lucian Radu Teodorescu" << endl << endl;

    boost::timer::auto_cpu_timer timer(3, "\nTime elapsed: %ws\n\n");

    try
    {
        initSettingsWithArgs(argc, argv);
    }
    catch(exception& ex)
    {
        cout << "Cannot read command line arguments: " << ex.what() << endl;
        cout << "Try executing 'SparrowCompiler --help' for help on command line parameters" << endl;
        return -1;
    }
    const auto& s = *Nest_compilerSettings();

    if ( s.printVersion_ )
    {
        return 1;
    }

    if ( s.filesToBeCompiled_.empty() )
    {
        cout << "No input file was given!" << endl;
        cout << "Try executing 'SparrowCompiler --help' for help on command line parameters" << endl;
        return 1;
    }

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

    try
    {
        doCompilation(modules);
    }
    catch (...)
    {
    }

    // Destroy the modules
    for ( CompilerModule* mod : modules )
    {
        if ( mod->destroyFun )
            mod->destroyFun();
    }

    return 0;
}
