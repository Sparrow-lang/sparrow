#include "Nest/src/StdInc.hpp"
#include "Nest/Api/Compiler.h"
#include "Nest/Utils/CompilerSettings.hpp"
#include "Nest/Utils/Alloc.h"
#include "Nest/Utils/Diagnostic.hpp"
#include "Nest/Utils/StringRef.hpp"
#include "Nest/Utils/NodeUtils.hpp"
#include "Nest/Api/Node.h"
#include "Nest/Api/NodeArray.h"
#include "Nest/Api/CompilationContext.h"
#include "Nest/Api/SourceCode.h"
#include "Nest/Api/SourceCodeKindRegistrar.h"
#include "Nest/Api/Backend.h"

#include <boost/filesystem.hpp>
#include <boost/unordered_set.hpp>
#include <boost/unordered_map.hpp>

using namespace boost::filesystem;

struct ImportInfo
{
    const SourceCode* originSourceCode_;
    boost::filesystem::path filename_;

    ImportInfo(const SourceCode* orig, StringRef filename)
        : originSourceCode_(orig), filename_(filename.begin)
    {}
};


CompilerSettings _settings;
CompilationContext* _rootContext;
Backend* _backend;

/// The path of the current directory
boost::filesystem::path _curPath;

/// List of source codes parsed by this class
unordered_map<const SourceCode*, boost::filesystem::path> _sourceCodes;

/// The files that were handled before
unordered_map<string, SourceCode*> _handledFiles;

/// List of source codes to compile
vector<SourceCode*> _toCompile;

/// The imports that were not handled, as no appropriate source code was found
/// We associate these imports to the originating source code
unordered_map<const SourceCode*, vector<ImportInfo> > _unhandledImports;

/// List of nodes to be semantically checked
NodeArray _toSemanticCheck;

vector<FSourceCodeCallback> _sourceCodeCreatedCallbacks;
vector<FSourceCodeCallback> _sourceCodeParsedCallbacks;
vector<FSourceCodeCallback> _sourceCodeCompiledCallbacks;
vector<FSourceCodeCallback> _sourceCodeCodeGenCallbacks;

void _executeCUCallbacks(const vector<FSourceCodeCallback>& callbacks, SourceCode* sc) {
    for ( auto cb: callbacks ) {
        if ( cb )
            (*cb)(sc);
    }
}

void _semanticCheckNodes()
{
    while ( Nest_nodeArraySize(_toSemanticCheck) > 0  )
    {
        Node* n = at(_toSemanticCheck, 0);
        Nest_eraseNodeFromArray(&_toSemanticCheck, 0);

        if ( n )
            Nest_semanticCheck(n);  // Ignore possible failures
    }
}

/// Check if a file with the exact name exists.
/// Makes sure the comparison of the filename is case sensitive
bool _fileExists(const path& f) {
    // First check if the the file exists
    if ( !is_regular_file(f) )
        return false;

    // Now we need to compare the filename case insensitive

    // Get the parent directory
    auto parentDir = f.parent_path();
    if ( parentDir.empty() )
        parentDir = current_path();

    // Check if the file entry in the parent directory matches our filename
    auto filename = f.filename();
    auto dirIt = directory_iterator(parentDir);
    for ( ; dirIt != directory_iterator(); ++dirIt ) {
        if ( dirIt->path().filename() == filename )
            return true;
    }

    return false;
}

pair<bool, SourceCode*> _handleImportFile(const ImportInfo& import)
{
    // Check if the file exists - if the file does not exist, exit and try another path
    if ( !_fileExists(import.filename_) ) {
        return make_pair(false, nullptr);
    }

    // Get the absolute path
    string absPath;
    absPath = canonical(import.filename_).string();

    // Check if this file was handled before
    auto it = _handledFiles.find(absPath);
    if ( it != _handledFiles.end() )
        return make_pair(true, it->second);

    // Try to create a source code for the input file
    const char* url = dupString(import.filename_.string().c_str());
    int scKind = Nest_getSourceCodeKindForFilename(url);
    if ( scKind < 0 )
    {
        _unhandledImports[import.originSourceCode_].push_back(import);
        return make_pair(true, nullptr);
    }
    SourceCode* sourceCode = (SourceCode*) alloc(sizeof(SourceCode), allocGeneral);
    sourceCode->kind = scKind;
    sourceCode->url = url;
    _sourceCodes.insert(make_pair(sourceCode, absPath));

    // Notify the listeners that a new source code was created
    _executeCUCallbacks(_sourceCodeCreatedCallbacks, sourceCode);

    // Mark this file as being handled
    _handledFiles[absPath] = sourceCode;

    int errorCount = Nest_getErrorsNum();

    // Create a new CompilationContext for the sourceCode
    CompilationContext* newContext = Nest_mkChildContext(_rootContext, modeUnspecified);
    newContext->sourceCode = sourceCode;

    // Do the parsing
//    REP_INFO(NOLOC, "Parsing: %1%") % import.filename_.string();
    Nest_parseSourceCode(sourceCode, newContext);

    // Notify the listeners that a new source code was parsed
    _executeCUCallbacks(_sourceCodeParsedCallbacks, sourceCode);

    // Stop if we have some (parsing) errors
    if ( errorCount != Nest_getErrorsNum() )
        return make_pair(true, sourceCode);

    // If we are not interesting only in syntax, compile the file
    if ( !_settings.syntaxOnly_ )
    {
        // We need to compile this source code
        _toCompile.push_back(sourceCode);
    }
    
    return make_pair(true, sourceCode);
}

SourceCode* _handleImport(const ImportInfo& import)
{
    pair<bool, SourceCode*> res;

    // Check if the filename path is absolute
    path p = import.filename_;
    if ( p.is_complete() )
    {
        res = _handleImportFile(import);
        if ( !res.first )
            REP_ERROR(NOLOC, "Cannot find input file: %1%") % import.filename_.string();
        return res.second;
    }

    // Try to use the original source code as base for the relative filename
    if ( import.originSourceCode_ )
    {
        auto it = _sourceCodes.find(import.originSourceCode_);
        if ( it != _sourceCodes.end() )
        {
            ImportInfo importRel = import;
            importRel.filename_ = it->second.parent_path() / p;
            res = _handleImportFile(importRel);
            if ( res.first )
                return res.second;
        }
    }

    // Test to see if the file is relative to some of the import paths
    for ( const string& importPath: _settings.importPaths_ )
    {
        ImportInfo importRel = import;
        importRel.filename_ = path(importPath) / p;
        res = _handleImportFile(importRel);
        if ( res.first )
            return res.second;
    }

    // Test to see if the file is relative to the current path
    ImportInfo importRel = import;
    importRel.filename_ = _curPath / p;
    res = _handleImportFile(importRel);

    if ( !res.first )
        REP_ERROR(NOLOC, "Cannot find input file: %1%") % import.filename_.string();
    return res.second;
}


void Nest_compilerInit()
{
    try
    {
        _curPath = boost::filesystem::current_path();
    }
    catch (const exception& e)
    {
        REP_INTERNAL(NOLOC, "Cannot obtain the current path; aborting (%1%") % e.what();
    }
}

CompilerSettings* Nest_compilerSettings()
{
    return &_settings;
}


CompilationContext* Nest_getRootContext()
{
    return _rootContext;
}

Backend* Nest_getCurBackend()
{
    return _backend;
}

void Nest_createBackend(const char* mainFilename)
{
    // Select the first available backend
    ASSERT(Nest_getNumBackends() > 0);
    _backend = Nest_getBackend(0);

    // Initialize the backend
    _backend->init(_backend, mainFilename);

    // Also create the root context
    _rootContext = Nest_mkRootContext(_backend, modeRt);
}

SourceCode* Nest_compileFile(StringRef filename)
{
    if ( size(filename) == 0 || filename.begin[0] == '\r' || filename.begin[0] == '\n' )
        return nullptr;

    if ( !_backend )
        REP_INTERNAL(NOLOC, "No backend set");

    // Parse the source code
    SourceCode* sc = Nest_addSourceCodeByFilename(nullptr, filename);

    if ( _settings.syntaxOnly_ )
        return sc;

    vector<SourceCode*> toCodeGenerate;

    // Compile everything we have to compile
    // One file might include multiple files, and therefore we may need to compile those too
    while ( !_toCompile.empty() )
    {
        // Pop a source code to compile it
        SourceCode* sourceCode = _toCompile.front();

        // If this source code has some unhanded includes, handle them now
        auto it = _unhandledImports.find(sourceCode);
        if ( it != _unhandledImports.end() )
        {
            auto& imports = it->second;
            if ( !imports.empty() )
            {
                // Handle all the imports
                for ( ImportInfo& ii: imports )
                {
                    auto res = _handleImportFile(ii);
                    if ( !res.first )
                        REP_ERROR(NOLOC, "Don't know how to interpret file %1%") % ii.filename_;
                }
                it->second.clear();
                continue;   // Restart the loop to process everything in order
            }
        }

        _toCompile.erase(_toCompile.begin());
        // REP_INFO(NOLOC, "Compiling %1%") % sourceCode->filename();

        int errorCount = Nest_getErrorsNum();

        // Semantic check the source code
        Nest_queueSemanticCheck(sourceCode->mainNode);
        _semanticCheckNodes();

        // Notify the listeners that the source code was compiled
        _executeCUCallbacks(_sourceCodeCompiledCallbacks, sourceCode);

        // Move to the next source code if we have some errors
        if ( errorCount != Nest_getErrorsNum() )
            continue;

        toCodeGenerate.push_back(sourceCode);
    }

    // Do the code generation in backend
    if ( !_settings.syntaxOnly_ )
    {
        for ( SourceCode* code: toCodeGenerate ) {
            _backend->generateMachineCode(_backend, code);

            // Notify the listeners that we've generated code for the source code
            _executeCUCallbacks(_sourceCodeCodeGenCallbacks, code);
        }
    }

    return sc;
}

SourceCode* Nest_addSourceCodeByFilename(const SourceCode* orig, StringRef filename)
{
    return _handleImport(ImportInfo(orig, filename));
}

const SourceCode* Nest_getSourceCodeForFilename(StringRef filename)
{
    path filepath = toString(filename);
    for ( auto p: _sourceCodes )
    {
        if ( p.second == filepath )
            return p.first;
    }
    return nullptr;
}

void Nest_queueSemanticCheck(Node* node)
{
    Nest_appendNodeToArray(&_toSemanticCheck, node);
}

void Nest_ctProcess(Node* node)
{
    node = Nest_semanticCheck(node);
    if ( node )
        _backend->ctProcess(_backend, node);
}

Node* Nest_ctEval(Node* node)
{
    node = Nest_semanticCheck(node);
    if ( !node )
        return nullptr;
    Node* res = _backend->ctEvaluate(_backend, node);
    if ( !res )
        return nullptr;
    Nest_setContext(res, node->context);
    return Nest_semanticCheck(res);
}

unsigned Nest_sizeOf(TypeRef type)
{
    return _backend->sizeOf(_backend, type);
}
unsigned Nest_alignmentOf(TypeRef type)
{
    return _backend->alignmentOf(_backend, type);
}

void Nest_registerSourceCodeCreatedCallback(FSourceCodeCallback callback)
{
    _sourceCodeCreatedCallbacks.push_back(callback);
}
void Nest_registerSourceCodeParsedCallback(FSourceCodeCallback callback)
{
    _sourceCodeParsedCallbacks.push_back(callback);
}
void Nest_registerSourceCodeCompiledCallback(FSourceCodeCallback callback)
{
    _sourceCodeCompiledCallbacks.push_back(callback);
}
void Nest_registerSourceCodeCodeGenCallback(FSourceCodeCallback callback)
{
    _sourceCodeCodeGenCallbacks.push_back(callback);
}
