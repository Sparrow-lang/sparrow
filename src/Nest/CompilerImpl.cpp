#include <StdInc.h>
#include "CompilerImpl.h"

#include <Common/Alloc.h>
#include <Common/Diagnostic.h>
#include <Common/DiagnosticReporter.h>
#include <Common/Serialization.h>
#include <Intermediate/Node.h>
#include <Intermediate/NodeSer.h>
#include <Intermediate/CompilationContext.h>
#include <Frontend/SourceCode.h>
#include <Frontend/SourceCodeKindRegistrar.h>
#include <Backend/Backend.h>

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
        Common::saveToBinFile(sc.mainNode, (filenameBase + ".out").c_str());
        Common::saveToJsonFile(sc.mainNode, (filenameBase + ".json").c_str());
    }
}

CompilerImpl::CompilerImpl()
    : diagnosticReporter_(new Common::DiagnosticReporter)
    , backend_(nullptr)
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
    delete diagnosticReporter_;
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

Backend& CompilerImpl::backend() const
{
    return *backend_;
}

void CompilerImpl::createBackend(const char* mainFilename)
{
    // Select the first available backend
    ASSERT(Nest_getNumBackends() > 0);
    backend_ = Nest_getBackend(0);

    // Initialize the backend
    backend_->init(backend_, mainFilename);

    // Also create the root context
    rootContext_ = new CompilationContext(*backend_);
    rootContext_->setEvalMode(modeRt);
}

void CompilerImpl::compileFile(const string& filename)
{
    if ( filename.empty() || filename[0] == '\r' || filename[0] == '\n' )
        return;

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
                queueSemanticCheck(sourceCode->mainNode);
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

        // Do the code generation in backend
        if ( !theCompiler().settings().syntaxOnly_ )
        {
            for ( SourceCode* code: toCodeGenerate )
                backend_->generateMachineCode(backend_, code);
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
    Nest::semanticCheck(node);
    backend_->ctProcess(backend_, node);
}

Node* CompilerImpl::ctEval(Node* node)
{
    Nest::semanticCheck(node);
    Node* res = backend_->ctEvaluate(backend_, node);
    if ( res )
    {
        Nest::setContext(res, node->context);
        Nest::semanticCheck(res);
    }
    return res;
}

size_t CompilerImpl::sizeOf(TypeRef type)
{
    return backend_->sizeOf(backend_, type);
}
size_t CompilerImpl::alignmentOf(TypeRef type)
{
    return backend_->alignmentOf(backend_, type);
}


void CompilerImpl::semanticCheckNodes()
{
    while ( !toSemanticCheck_.empty() )
    {
        Node* n = toSemanticCheck_.front();
        toSemanticCheck_.erase(toSemanticCheck_.begin());

        if ( n )
            Nest::semanticCheck(n);
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

    // Try to create a source code for the input file
    const char* url = dupString(import.filename_.string().c_str());
    int scKind = Nest_getSourceCodeKindForFilename(url);
    if ( scKind < 0 )
    {
        unhandledImports_[import.originSourceCode_].push_back(import);
        return false;
    }
    SourceCode* sourceCode = (SourceCode*) alloc(sizeof(SourceCode), allocGeneral);
    sourceCode->kind = scKind;
    sourceCode->url = url;
    sourceCodes_.insert(make_pair(sourceCode, absPath));

    // Mark this file as being handled
    handledFiles_.insert(absPath);

    int errorCount = theCompiler().diagnosticReporter().errorsCount();

    // Create a new CompilationContext for the sourceCode
    CompilationContext* newContext = new CompilationContext(rootContext_);
    newContext->setSourceCode(sourceCode);

    // Do the parsing
//    REP_INFO(NOLOC, "Parsing: %1%") % import.filename_.string();
    Nest_parseSourceCode(sourceCode, newContext);

    // Dump the content of the file, before compiling it
    if ( settings_.dumpAST_ )
        dumpAst(*sourceCode, false);

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
            //checkQidAgainstUnit(sourceCode, qid);
        }

        // We need to compile this source code
        toCompile_.push_back(sourceCode);
    }
    
    return true;
}
