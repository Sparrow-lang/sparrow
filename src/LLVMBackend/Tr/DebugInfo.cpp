#include <StdInc.h>
#include "DebugInfo.h"
#include "TrContext.h"

#include "Feather/Utils/FeatherUtils.hpp"

#include "Nest/Utils/Diagnostic.hpp"
#include "Nest/Utils/StringRef.hpp"
#include "Nest/Api/SourceCode.h"
#include "Nest/Api/Node.h"

#include <boost/filesystem.hpp>

using namespace LLVMB;
using namespace LLVMB::Tr;
using namespace Nest;
using namespace std;

namespace {
pair<string, string> splitFilename(const char* path) {
    auto absPath = boost::filesystem::canonical(path);
    auto filename = absPath.filename().string();
    auto dir = absPath.parent_path().string();
    return make_pair(dir, filename);
}

llvm::DebugLoc getDebugLoc(const Location& loc, llvm::MDNode* scope, bool takeStart = true) {
    size_t line = takeStart ? loc.start.line : loc.end.line;
    size_t col = takeStart ? loc.start.col : loc.end.col;
    return llvm::DebugLoc::get(line, col, scope);
}
} // namespace

DebugInfo::DebugInfo(llvm::Module& module, const string& mainFilename)
    : diBuilder_(module)
    , compileUnit_(nullptr) {
    // Make sure that we set the debug info version to the module
    module.addModuleFlag(llvm::Module::Error, "Debug Info Version", llvm::DEBUG_METADATA_VERSION);

    // Create a compile-unit for the module
    createCompileUnit(mainFilename);
}

DebugInfo::~DebugInfo() = default;

void DebugInfo::finalize() { diBuilder_.finalize(); }

void DebugInfo::emitLocation(LlvmBuilder& builder, const Location& loc, bool takeStart) {
    // Update our current location
    setLocation(builder, loc);
    if (Nest_isLocEmpty(&curLoc_) || 0 == Nest_compareLocations(&curLoc_, &prevLoc_))
        return;

    prevLoc_ = curLoc_;

    ASSERT(!lexicalBlockStack_.empty());
    llvm::MDNode* scope = lexicalBlockStack_.back();
    if (scope)
        builder.SetCurrentDebugLocation(getDebugLoc(curLoc_, scope, takeStart));
}

void DebugInfo::emitFunctionStart(LlvmBuilder& builder, Node* fun, llvm::Function* llvmFun) {
    regionCountAtFunStartStack_.push_back(lexicalBlockStack_.size());

    const Location& loc = fun->location;
    llvm::DIFile* file = getOrCreateFile(fun->location);
    llvm::DIScope* fContext = file;

    // For now, just create a fake subroutine type -- it should be ok
    llvm::DISubroutineType* diFunType = createFunctionType(llvmFun);

    StringRef name = Nest_hasProperty(fun, "name") ? Feather_getName(fun) : fromCStr("anonymous");
    llvm::StringRef nameLLVM(name.begin, size(name));
    llvm::DISubprogram* diSubprogram = diBuilder_.createFunction(fContext, // function scope
            nameLLVM,                                                      // function name
            llvmFun->getName(),           // mangled function name (link name)
            file,                         // file where this is defined
            loc.start.line,               // line number
            diFunType,                    // function type
            false,                        // true if this function is not externally visible
            true,                         // is a function definition
            loc.start.line,               // the beginning of the scope this starts
            llvm::DINode::FlagPrototyped, // flags
            false);                       // is optimized

    llvmFun->setSubprogram(diSubprogram);

    // Push function on region stack.
    lexicalBlockStack_.push_back(diSubprogram);
    regionMap_[fun] = diSubprogram;
}

void DebugInfo::emitFunctionEnd(LlvmBuilder& builder, const Location& loc) {
    ASSERT(!regionCountAtFunStartStack_.empty());
    unsigned regionCount = regionCountAtFunStartStack_.back();
    ASSERT(regionCount <= lexicalBlockStack_.size());

    // Pop all regions for this function.
    while (lexicalBlockStack_.size() != regionCount)
        emitLexicalBlockEnd(builder, loc);
    regionCountAtFunStartStack_.pop_back();
}

void DebugInfo::emitLexicalBlockStart(LlvmBuilder& builder, const Location& loc) {
    // Set the current location
    setLocation(builder, loc);

    ASSERT(!lexicalBlockStack_.empty());

    llvm::DILexicalBlock* lexicalBock = diBuilder_.createLexicalBlock(
            lexicalBlockStack_.back(), getOrCreateFile(loc), loc.start.line, loc.start.col);
    lexicalBlockStack_.push_back(lexicalBock);

    // Emit the debug location change
    builder.SetCurrentDebugLocation(getDebugLoc(curLoc_, lexicalBock));
}

void DebugInfo::emitLexicalBlockEnd(LlvmBuilder& builder, const Location& loc) {
    ASSERT(!lexicalBlockStack_.empty() && "Region stack mismatch, stack empty!");

    // Write the end of the lexical block in the line table
    emitLocation(builder, loc, false);

    lexicalBlockStack_.pop_back();
}

void DebugInfo::createCompileUnit(const string& mainFilename) {
    auto p = splitFilename(mainFilename.c_str());
    llvm::DIFile* file = diBuilder_.createFile(p.second, p.first);

    // Create new compile unit
    compileUnit_ = diBuilder_.createCompileUnit(llvm::dwarf::DW_LANG_C, // language
            file,                                                       // file name
            "Sparrow Compiler", // producer of debug information
            false,              // isOptimized
            "",                 // debug flags (command line options)
            1);                 // run-time version of the language
}

void DebugInfo::setLocation(LlvmBuilder& builder, const Location& loc) {
    // If the new location isn't valid return.
    if (Nest_isLocEmpty(&loc))
        return;

    curLoc_ = loc;

    // If we changed the files in the middle of a lexical scope, create a new lexical scope
    if (!Nest_isLocEmpty(&curLoc_) && Nest_isLocEmpty(&prevLoc_) &&
            curLoc_.sourceCode != prevLoc_.sourceCode && !lexicalBlockStack_.empty()) {
        llvm::DIScope* scope =
                lexicalBlockStack_.empty() ? compileUnit_ : lexicalBlockStack_.back();

        builder.SetCurrentDebugLocation(llvm::DebugLoc::get(loc.start.line, loc.start.col, scope));
    }
}

llvm::DIFile* DebugInfo::getOrCreateFile(const Location& loc) {
    // Check the cache first
    auto it = filenameCache_.find(loc.sourceCode);
    if (it != filenameCache_.end())
        return it->second;

    // Create the file, and cache it
    // If the location is not valid, then return the main file
    llvm::DIFile* file = nullptr;
    if (Nest_isLocEmpty(&loc)) {
        file = diBuilder_.createFile(compileUnit_->getFilename(), compileUnit_->getDirectory());
    } else {
        auto p = splitFilename(loc.sourceCode->url);
        file = diBuilder_.createFile(p.second, p.first);
    }
    filenameCache_.insert(make_pair(loc.sourceCode, file));
    return file;
}

const char* getIntTypeName(llvm::Type* t) {
    switch (t->getPrimitiveSizeInBits()) {
    case 1:
        return "i1";
    case 8:
        return "i8";
    case 16:
        return "i16";
    case 32:
        return "i32";
    case 64:
        return "i64";
    case 128:
        return "i128";
    default:
        return "int";
    }
}

llvm::DIType* transformType(llvm::DIBuilder& diBuilder, llvm::Type* t) {
    if (t) {
        if (t->isIntegerTy())
            return diBuilder.createBasicType(
                    getIntTypeName(t), t->getPrimitiveSizeInBits(), llvm::dwarf::DW_ATE_signed);
        else if (t->isFloatTy())
            return diBuilder.createBasicType(
                    "float", t->getPrimitiveSizeInBits(), llvm::dwarf::DW_ATE_float);
        else if (t->isDoubleTy())
            return diBuilder.createBasicType(
                    "double", t->getPrimitiveSizeInBits(), llvm::dwarf::DW_ATE_float);
        else if (t->isFloatingPointTy())
            return diBuilder.createBasicType(
                    "double", t->getPrimitiveSizeInBits(), llvm::dwarf::DW_ATE_float);
        else if (t->isVoidTy())
            return diBuilder.createBasicType("void", 0, llvm::dwarf::DW_ATE_unsigned);
        // else if (t->isStructTy()) // TODO
        //     return diBuilder.createStructType("void", 0, llvm::dwarf::DW_ATE_unsigned);
        else if (t->isArrayTy()) {
            auto baseType = transformType(diBuilder, t->getArrayElementType());
            llvm::DINodeArray subscripts;
            return diBuilder.createArrayType(t->getArrayNumElements(), 0, baseType, subscripts);
        } else if (t->isPointerTy()) {
            auto baseType = transformType(diBuilder, t->getPointerElementType());
            return diBuilder.createPointerType(baseType, t->getPrimitiveSizeInBits());
        } else if (t->isFunctionTy()) {
            llvm::SmallVector<llvm::Metadata*, 8> funTypes;
            auto num = t->getNumContainedTypes();
            for (unsigned i = 0; i < num; i++)
                funTypes.push_back(transformType(diBuilder, t->getContainedType(i)));
            return diBuilder.createSubroutineType(diBuilder.getOrCreateTypeArray(funTypes));
        }
    }

    // Last resort
    return diBuilder.createBasicType("void", 0, llvm::dwarf::DW_ATE_unsigned);
}

llvm::DISubroutineType* DebugInfo::createFunctionType(llvm::Function* llvmFun) {
    // TODO: Try to cache the types that we are using
    // NOLINTNEXTLINE
    return (llvm::DISubroutineType*)transformType(diBuilder_, llvmFun->getFunctionType());
}
