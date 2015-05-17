#include <StdInc.h>
#include "CompilerImpl.h"

#include <Common/Diagnostic.h>
#include <Common/DiagnosticReporter.h>
#include <Common/NodeAllocatorImpl.h>
#include <Common/TimingSystem.h>
#include <Common/Serialization.h>
#include <Intermediate/Node.h>
#include <Intermediate/CompilationContext.h>
#include <Frontend/SourceCode.h>
#include <Frontend/FrontendFactoryImpl.h>
#include <Backend/Backend.h>
#include <Backend/BackendFactoryImpl.h>

using namespace Nest;
using namespace std;
using namespace boost::filesystem;

namespace
{
    void dumpAst(SourceCode& sc, bool isCompiled)
    {
        // Note: at this stage we keep overwriting the dumps we have written so far
        // At this point, because we process a lot of files and we are actually
        // interested in the last file, this is a feature, not a bug.
        string filenameBase = isCompiled ? "nodesComp" : "nodesOrig";
        Common::saveToBinFile(sc.iCode(), (filenameBase + ".out").c_str());
        Common::saveToJsonFile(sc.iCode(), (filenameBase + ".json").c_str());
    }
}

CompilerImpl::CompilerImpl()
    : diagnosticReporter_(new Common::DiagnosticReporter)
    , frontendFactory_(new FrontendFactoryImpl)
    , backendFactory_(new BackendFactoryImpl)
    , backend_(nullptr)
    , nodeAllocator_(new Common::NodeAllocatorImpl)
    , timingSystem_(new Common::TimingSystem)
{
    try
    {
	    curPath_ = boost::filesystem::current_path();
    }
    catch (const exception& e)
    {
        REP_INTERNAL(NOLOC, "Cannot obtain the current path; aborting (%1%") % e.what();
        exit(-1);
    }
}

CompilerImpl::~CompilerImpl()
{
    // Remove all the source codes
    for ( auto& p: sourceCodes_ )
        delete p.first;

    delete timingSystem_;
    delete diagnosticReporter_;
    delete frontendFactory_;
    delete backendFactory_;
    delete backend_;
}

CompilerSettings& CompilerImpl::settings()
{
    return settings_;
}

const CompilerSettings& CompilerImpl::settings() const
{
    return settings_;
}


Common::DiagnosticReporter& CompilerImpl::diagnosticReporter() const
{
    return *diagnosticReporter_;
}

CompilationContext* CompilerImpl::rootContext() const
{
    return rootContext_;
}

FrontendFactory& CompilerImpl::frontendFactory() const
{
    return *frontendFactory_;
}

BackendFactory& CompilerImpl::backendFactory() const
{
    return *backendFactory_;
}

Backend& CompilerImpl::backend() const
{
    return *backend_;
}

Common::NodeAllocator& CompilerImpl::nodeAllocator() const
{
    return *nodeAllocator_;
}

Common::TimingSystem* CompilerImpl::timingSystem() const
{
    return timingSystem_;
}

void CompilerImpl::createBackend(const string& backendName)
{
    delete backend_;
    backend_ = backendFactory_->createBackend(backendName);

    // Also create the root context
    rootContext_ = new CompilationContext(*backend_);
    rootContext_->setEvalMode(modeRt);
}

void CompilerImpl::compileFile(const string& filename)
{
    if ( filename.empty() || filename[0] == '\r' || filename[0] == '\n' )
        return;

    ENTER_TIMER_DESC(theCompiler().timingSystem(), "comp", "File compilation");

    if ( !backend_ )
        REP_INTERNAL(NOLOC, "No backend set");

    try
    {
        // Parse the source code
        addSourceCodeByFilename(nullptr, filename);

        if ( theCompiler().settings().syntaxOnly_ )
            return;

        vector<SourceCode*> toCodeGenerate;

        {
            ENTER_TIMER_DESC(theCompiler().timingSystem(), "comp.2semanticChec", "  Semantic checking");

            // Compile everything we have to compile
            // One file might include multiple files, and therefore we may need to compile those too
            while ( !toCompile_.empty() )
            {
                // Pop a source code to compile it
                SourceCode* sourceCode = toCompile_.front();

                // If this source code has some unhandled includes, handle them now
                auto it = unhandledImports_.find(sourceCode);
                if ( it != unhandledImports_.end() )
                {
                    auto& imports = it->second;
                    if ( !imports.empty() )
                    {
                        // Handle all the imports
                        for ( ImportInfo& ii: imports )
                        {
                            if ( !handleImportFile(ii) )
                                REP_ERROR(NOLOC, "Don't know how to interpret file %1%") % ii.filename_;
                        }
                        it->second.clear();
                        continue;   // Restart the loop to process everything in order
                    }
                }

                toCompile_.erase(toCompile_.begin());
//                REP_INFO(NOLOC, "Compiling %1%") % sourceCode->filename();

                int errorCount = theCompiler().diagnosticReporter().errorsCount();

                // Semantic check the source code
                queueSemanticCheck(sourceCode->iCode());
                semanticCheckNodes();

                // Move to the next source code if we have some errors
                if ( errorCount != theCompiler().diagnosticReporter().errorsCount() )
                    continue;

                // Dump the content of the file, after it was compiled
                if ( settings_.dumpAST_ )
                    dumpAst(*sourceCode, true);
                
                toCodeGenerate.push_back(sourceCode);
            }
        }

        ENTER_TIMER_DESC(theCompiler().timingSystem(), "comp.3backend", "  Backend processing");

        // Do the code generation in backend
        if ( !theCompiler().settings().syntaxOnly_ )
        {
            for ( SourceCode* code: toCodeGenerate )
                backend_->generateMachineCode(*code);
        }
    }
    catch (...)
    {
        // Don't pass exceptions upward
    }
}

void CompilerImpl::addSourceCodeByFilename(const SourceCode* orig, string filename)
{
    handleImport(ImportInfo(orig, move(filename)));
}

void CompilerImpl::addSourceCodeByQid(const SourceCode* orig, vector<string> qid)
{
    if ( qid.empty() )
        REP_INTERNAL(NOLOC, "Nothing to import");

    // Convert qid into a filename
    string filename;
    for ( const string& part: qid )
    {
        if ( part.empty() )
            continue;

        if ( !filename.empty() )
            filename += "/";
        filename += part;
    }

    if ( qid.back().empty() )
        handleImport(ImportInfo(orig, move(filename), move(qid)));      // Star notation
    else
        handleImport(ImportInfo(orig, filename + ".spr"));
}

const SourceCode* CompilerImpl::getSourceCodeForFilename(const string& filename)
{
    path filepath = filename;
    for ( auto p: sourceCodes_ )
    {
        if ( p.second == filepath )
            return p.first;
    }
    return nullptr;
}

void CompilerImpl::queueSemanticCheck(Node* node)
{
    toSemanticCheck_.push_back(node);
}

void CompilerImpl::ctProcess(Node* node)
{
    node->semanticCheck();
    backend_->ctProcess(node);
}

Node* CompilerImpl::ctEval(Node* node)
{
    node->semanticCheck();
    Node* res = backend_->ctEvaluate(node);
    if ( res )
    {
        res->setContext(node->context());
        res->semanticCheck();
    }
    return res;
}

size_t CompilerImpl::sizeOf(Type* type)
{
    return backend_->sizeOf(type);
}
size_t CompilerImpl::alignmentOf(Type* type)
{
    return backend_->alignmentOf(type);
}


void CompilerImpl::semanticCheckNodes()
{
    while ( !toSemanticCheck_.empty() )
    {
        Node* n = toSemanticCheck_.front();
        toSemanticCheck_.erase(toSemanticCheck_.begin());

        if ( n )
            n->semanticCheck();
    }
}


void CompilerImpl::handleImport(const ImportInfo& import)
{
    // Check if the filename path is absolute
    path p = import.filename_;
    if ( p.is_complete() )
    {
        if ( !checkFileDir(import) )
            REP_ERROR(NOLOC, "Cannot find input file: %1%") % import.filename_.string();
        return;
    }

    // Try to use the original source code as base for the relative filename
    if ( import.originSourceCode_ )
    {
        auto it = sourceCodes_.find(import.originSourceCode_);
        if ( it != sourceCodes_.end() )
        {
            ImportInfo importRel = import;
            importRel.filename_ = it->second.parent_path() / p;
            if ( checkFileDir(importRel) )
                return;
        }
    }

    // Test to see if the file is relative to some of the import paths
    for ( const string& importPath: theCompiler().settings().importPaths_ )
    {
        ImportInfo importRel = import;
        importRel.filename_ = path(importPath) / p;
        if ( checkFileDir(importRel) )
            return;
    }

    // Test to see if the file is relative to the current path
    ImportInfo importRel = import;
    importRel.filename_ = curPath_ / p;
    if ( checkFileDir(importRel) )
        return;

    REP_ERROR(NOLOC, "Cannot find input file: %1%") % import.filename_.string();
}

bool CompilerImpl::checkFileDir(const ImportInfo& import)
{
    // Check if the file/directory exists - if the file does not exist, exit and try another path
    if ( !exists(import.filename_) )
        return false;

    // If we have a directory, with a star, load all the files from the directory
    if ( is_directory(import.filename_) )
    {
        // If we are not looking for a directly, return false to try another path
        if ( import.qid_.empty() || !import.qid_.back().empty() )
            return false;

        handleDirectory(import);
        return true;
    }

    // Regular file
    handleImportFile(import);
    return true;
}

void CompilerImpl::handleDirectory(const ImportInfo& import)
{
    directory_iterator ite;
    for ( directory_iterator it(import.filename_); it != ite; ++it )
    {
        // Recursively call this function for subdirectories
        if ( is_directory(it->status()) )
        {
            // Ignore directories that start with '.'
            const string& dirName = it->path().filename().string();
            if ( !dirName.empty() && dirName[0] != '.' )
            {
                ImportInfo importSub = import;
                importSub.filename_ = it->path();
                importSub.qid_.push_back(dirName);
                handleDirectory(importSub);
            }
        }
        else if ( it->path().extension().string() == ".spr" )
        {
            ImportInfo importSub = import;
            importSub.filename_ = it->path();
            importSub.qid_.push_back(it->path().filename().string());
            handleImportFile(importSub);
        }
    }
}

bool CompilerImpl::handleImportFile(const ImportInfo& import)
{
    // Get the absolute path
    string absPath;
    absPath = system_complete(import.filename_).string();

    // Check if this file was handled before
    if ( handledFiles_.find(absPath) != handledFiles_.end() )
        return true;

    ENTER_TIMER_DESC(theCompiler().timingSystem(), "comp.1parsing", "  Parsing source code");

    // Try to create a parser for the input file
    SourceCode* parser = theCompiler().frontendFactory().createParser(import.filename_.string());
    if ( !parser )
    {
        unhandledImports_[import.originSourceCode_].push_back(import);
        return false;
    }
    sourceCodes_.insert(make_pair(parser, absPath));

    // Mark this file as being handled
    handledFiles_.insert(absPath);

    int errorCount = theCompiler().diagnosticReporter().errorsCount();

    // Create a new CompilationContext for the parser
    CompilationContext* newContext = new CompilationContext(rootContext_);
    newContext->setSourceCode(parser);

    // Do the parsing
//    REP_INFO(NOLOC, "Parsing: %1%") % import.filename_.string();
    parser->parse(newContext);

    // Dump the content of the file, before compiling it
    if ( settings_.dumpAST_ )
        dumpAst(*parser, false);

    // Stop if we have some (parsing) errors
    if ( errorCount != theCompiler().diagnosticReporter().errorsCount() )
        return true;

    // If we are not interesting only in syntax, compile the file
    if ( !theCompiler().settings().syntaxOnly_ )
    {
        // If we have an import that contains a namespace part, make sure that is the same as the package
        if ( import.qid_.size() > 1 )
        {
            // TODO (import): Implement this
            //checkQidAgainstUnit(parser, qid);
        }

        // We need to compile this source code
        toCompile_.push_back(parser);
    }
    
    return true;
}
