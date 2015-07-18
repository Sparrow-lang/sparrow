#include <StdInc.h>
#include "DebugInfo.h"
#include "TrContext.h"

#include <Feather/Nodes/Decls/Function.h>
#include <Feather/Util/Decl.h>

#include <Nest/Common/Diagnostic.h>
#include <Nest/Frontend/SourceCode.h>

#include <boost/filesystem.hpp>

using namespace LLVMB;
using namespace LLVMB::Tr;
using namespace Nest;
using namespace std;

namespace
{
    pair<string, string> splitFilename(const string& fullFilename)
    {
        boost::filesystem::path p(fullFilename);
        return make_pair(p.parent_path().string(), p.filename().string());
    }

    llvm::DebugLoc getDebugLoc(const Location& loc, llvm::MDNode* scope, bool takeStart = true)
    {
        size_t line = takeStart ? loc.startLineNo() : loc.endLineNo();
        size_t col = takeStart ? loc.startColNo() : loc.endColNo();
        return llvm::DebugLoc::get(line, col, scope);
    }
}

DebugInfo::DebugInfo(llvm::Module& module, const string& mainFilename)
    : diBuilder_(module)
{
    // Make sure that we set the debug info version to the module
    module.addModuleFlag(llvm::Module::Error, "Debug Info Version", llvm::DEBUG_METADATA_VERSION);

    // Create a compile-unit for the module
    createCompileUnit(mainFilename);
}

DebugInfo::~DebugInfo()
{
}

void DebugInfo::finalize()
{
    diBuilder_.finalize();
}

void DebugInfo::emitLocation(LlvmBuilder& builder, const Location& loc, bool takeStart)
{
    // Update our current location
    setLocation(loc);
    if ( curLoc_.empty() || curLoc_ == prevLoc_ )
        return;

    prevLoc_ = curLoc_;

    ASSERT(!lexicalBlockStack_.empty());
    llvm::MDNode* scope = lexicalBlockStack_.back();
    if ( scope )
        builder.SetCurrentDebugLocation(getDebugLoc(curLoc_, scope, takeStart));
}

void DebugInfo::emitFunctionStart(LlvmBuilder& builder, Feather::Function* fun, llvm::Function* llvmFun)
{
    regionCountAtFunStartStack_.push_back(lexicalBlockStack_.size());

    const Location& loc = fun->location();

    llvm::DIFile file = getOrCreateFile(fun->location());
    llvm::DIDescriptor fileDesc(file);

    // For now, just create an empty function declaration
    llvm::DISubprogram diFunDecl;

    // For now, just create a fake subroutine type -- it should be ok
    llvm::DICompositeType diFunType = diBuilder_.createSubroutineType(file, diBuilder_.getOrCreateArray({}));

    // Debug info for function declaration

    llvm::DISubprogram diSubprogram;
    diSubprogram = diBuilder_.createFunction(
        fileDesc,                   // function scope
        Feather::getName(fun->node()),      // function name
        llvmFun->getName(),         // mangled function name (link name)
        file,                       // file where this is defined
        loc.startLineNo(),          // line number
        diFunType,                  // function type
        true,                       // true if this function is not externally visible
        true,                       // is a function definition
        loc.startLineNo(),          // the beginning of the scope this starts
        0,                          // flags
        false,                      // is optimized
        llvmFun,                    // llvm::Function pointer
        nullptr,                    // function template parameters
        diFunDecl);                 // decl
    if ( !diSubprogram.Verify() )
        REP_INTERNAL(NOLOC, "Invalid debug metadata generated for function");

    // Push function on region stack.
    lexicalBlockStack_.push_back((llvm::MDNode*) diSubprogram);
    regionMap_[(Feather::Decl*)fun] = llvm::WeakVH(diSubprogram);
}

void DebugInfo::emitFunctionEnd(LlvmBuilder& builder, const Location& loc)
{
    ASSERT(!regionCountAtFunStartStack_.empty());
    unsigned regionCount = regionCountAtFunStartStack_.back();
    ASSERT(regionCount <= lexicalBlockStack_.size());

    // Pop all regions for this function.
    while ( lexicalBlockStack_.size() != regionCount)
        emitLexicalBlockEnd(builder, loc);
    regionCountAtFunStartStack_.pop_back();
}

void DebugInfo::emitLexicalBlockStart(LlvmBuilder& builder, const Location& loc)
{
    // Set the current location
    setLocation(loc);

    // Create a debug information metadata for the lexical block
    llvm::DIDescriptor scope = lexicalBlockStack_.empty()
        ? llvm::DIDescriptor()
        : llvm::DIDescriptor(lexicalBlockStack_.back());
    llvm::DIDescriptor desc = diBuilder_.createLexicalBlock(
        scope,                      // the scope of this lexical block
        getOrCreateFile(loc),       // the file of this lexical block
        loc.startLineNo(),          // the start line number of this lexical block
        loc.startColNo(),           // the start column number of the lexical block
        0);                         // DRAWF path discriminator
    llvm::MDNode* node = desc;
    lexicalBlockStack_.push_back(node);

    // Emit the debug location change
    builder.SetCurrentDebugLocation(getDebugLoc(curLoc_, scope));

}

void DebugInfo::emitLexicalBlockEnd(LlvmBuilder& builder, const Location& loc)
{
    ASSERT(!lexicalBlockStack_.empty() && "Region stack mismatch, stack empty!");

    // Write the end of the lexical block in the line table
    emitLocation(builder, loc, false);

    lexicalBlockStack_.pop_back();
}


void DebugInfo::createCompileUnit(const string& mainFilename)
{
    auto p = splitFilename(mainFilename);

    // Create new compile unit
    compileUnit_ = diBuilder_.createCompileUnit(
        llvm::dwarf::DW_LANG_lo_user + 109, // language
        p.second,                           // file name
        p.first,                            // directory
        "Sparrow Compiler",                 // producer of debug information
        false,                              // isOptimized
        "",                                 // debug flags (command line options)
        1,                                  // run-time version of the language
        "",                                 // split name
        llvm::DIBuilder::LineTablesOnly);   // type of debug generation
    if ( !compileUnit_.Verify() )
        REP_INTERNAL(NOLOC, "Invalid debug metadata generated for compile unit");
}

void DebugInfo::setLocation(const Location& loc)
{
    // If the new location isn't valid return.
    if ( loc.empty() )
        return;

    curLoc_ = loc;

    // If we changed the files in the middle of a lexical scope, create a new lexical scope
    if ( !curLoc_.empty() && prevLoc_.empty() && curLoc_.sourceCode() != prevLoc_.sourceCode() && !lexicalBlockStack_.empty() )
    {
        llvm::MDNode* lexicalBlock = lexicalBlockStack_.back();
        llvm::DIScope scope = llvm::DIScope(lexicalBlock);
        if ( scope.isLexicalBlockFile() )
        {
            llvm::DILexicalBlockFile lexicalBlockFile = llvm::DILexicalBlockFile(lexicalBlock);
            scope = lexicalBlockFile.getScope();
        }

        if ( scope.isLexicalBlock() )
        {
            llvm::DIDescriptor desc = diBuilder_.createLexicalBlockFile(scope, getOrCreateFile(curLoc_));
            if ( !desc.Verify() )
                REP_INTERNAL(NOLOC, "Invalid debug metadata generated for location");
            llvm::MDNode* node = desc;
            lexicalBlockStack_.pop_back();
            lexicalBlockStack_.push_back(node);
        }
    }
}

llvm::DIFile DebugInfo::getOrCreateFile(const Location& loc)
{
    // If the location is not valid, then return the main file
    if ( loc.empty() )
        return diBuilder_.createFile(compileUnit_.getFilename(), compileUnit_.getDirectory());

    // Check the cache first
    auto it = filenameCache_.find(loc.sourceCode());
    if ( it != filenameCache_.end() )
    {
        llvm::Value* val = it->second;
        ASSERT(val);
        return llvm::DIFile(llvm::cast<llvm::MDNode>(val));
    }

    // Create the file, and cache it
    auto p = splitFilename(loc.sourceCode()->filename());
    llvm::DIFile file = diBuilder_.createFile(p.second, p.first);
    filenameCache_.insert(make_pair(loc.sourceCode(), file));
    return file;
}
