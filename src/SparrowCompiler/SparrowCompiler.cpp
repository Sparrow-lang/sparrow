#include <StdInc.h>
#include <Settings.h>

#include <Nest/CompilerInstance.h>
#include <Nest/Compiler.h>
#include <Nest/CompilerSettings.h>
#include <Nest/Common/Diagnostic.h>
#include <Nest/Common/TimingSystem.h>
#include <Nest/Common/PrintTimer.h>
#include <Nest/Backend/Backend.h>
#include <Nest/Backend/BackendFactory.h>
#include <Nest/Intermediate/CompilationContext.h>

#include <Feather/Frontend/FSimpleSourceCode.h>
#include <Feather/CtApiFunctions.h>
#include <LLVMBackend/LLVMSourceCode.h>
#include <LLVMBackend/LLVMBackend.h>
#include <SparrowFrontend/SparrowSourceCode.h>
#include <SparrowFrontend/Helpers/StdDef.h>
#include <SparrowFrontend/CtApiFunctions.h>

#include <boost/filesystem.hpp>
#include <boost/scoped_ptr.hpp>

using namespace Nest;

namespace fs = boost::filesystem;

bool tryImplicitLibPath(const char* relPath)
{
    auto& s = theCompiler().settings();

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
    const auto& s = theCompiler().settings();
    if ( s.implicitLibFilePath_ != "auto" )
        return true;

    return tryImplicitLibPath("/SparrowImplicitLib")
        || tryImplicitLibPath("/../SparrowImplicitLib")
        || tryImplicitLibPath("/../../SparrowImplicitLib");
}

void doCompilation()
{
    // Set the LLVM backend
    theCompiler().createBackend("llvm");

    const auto& s = theCompiler().settings();

    // Initialize the backend
    ASSERT(!s.filesToBeCompiled_.empty());
    theCompiler().backend().init(s.filesToBeCompiled_[0]);

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

        // Initialize the Type type before loading anything
        SprFrontend::initTypeType(theCompiler().rootContext());

        // Register the CT API functions
        Feather::registerCtApiFunctions(theCompiler().backend());
        SprFrontend::registerCtApiFunctions(theCompiler().backend());

        // Process the implicit definitions file
        if ( !s.implicitLibFilePath_.empty() )
            theCompiler().compileFile(s.implicitLibFilePath_);
    }


    // Parse each individual file
    for ( const auto& filename: s.filesToBeCompiled_  )
    {
        Nest::Common::PrintTimer timer(s.verbose_, "", "   [%ws]\n");
        cout << filename;
        theCompiler().compileFile(filename);
        if ( !s.verbose_ )
            cout << endl;
    }

    // If we have no errors, start linking
    if ( !theCompiler().diagnosticReporter().wasError() && !s.syntaxOnly_ )
    {
        try
        {
            Nest::Common::PrintTimer timer(s.verbose_, "", "[%ws]\n");
            cout << "Linking..." << endl;
        	theCompiler().backend().link(outFilename);
        }
        catch (...)
        {
        }
    }
};

int main(int argc,char* argv[])
{
    cout << "Sparrow Compiler v0.9.3, (c) 2015 Lucian Radu Teodorescu" << endl << endl;

    boost::timer::auto_cpu_timer timer(3, "\nTime elapsed: %ws\n\n");

    CompilerInstance::instance().reset();

    {
        ENTER_TIMER_DESC(theCompiler().timingSystem(), "", "Compiler execution");

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
        const auto& s = theCompiler().settings();

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

        Feather::FSimpleSourceCode::registerSelf();
        LLVMB::LLVMSourceCode::registerSelf();
        LLVMB::LLVMBackend::registerSelf();
        SprFrontend::SparrowSourceCode::registerSelf();

        try
        {
            doCompilation();
        }
        catch (...)
        {
        }

        theCompiler().timingSystem()->dump();
    }

    CompilerInstance::instance().destroy();

    return 0;
}
