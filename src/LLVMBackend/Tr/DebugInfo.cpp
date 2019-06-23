#include <StdInc.h>
#include "DebugInfo.h"
#include "TrContext.h"
#include "TrType.h"
#include "GlobalContext.h"

#include "Feather/Utils/FeatherUtils.hpp"
#include "Feather/Utils/cppif/FeatherTypes.hpp"

#include "Nest/Utils/Diagnostic.hpp"
#include "Nest/Utils/cppif/StringRef.hpp"
#include "Nest/Utils/cppif/NodeUtils.hpp"
#include "Nest/Api/SourceCode.h"
#include "Nest/Api/Node.h"

#include <boost/filesystem.hpp>

using namespace LLVMB;
using namespace LLVMB::Tr;
using namespace Nest;
using namespace Feather;
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

llvm::StringRef toLlvm(StringRef s) { return {s.begin, (size_t)s.size()}; }

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

void DebugInfo::emitFunctionStart(GlobalContext& ctx, LlvmBuilder& builder, Node* fun,
        llvm::Function* llvmFun, llvm::BasicBlock* bodyBlock) {
    regionCountAtFunStartStack_.push_back(lexicalBlockStack_.size());

    const Location& loc = fun->location;
    llvm::DIFile* file = getOrCreateFile(fun->location);
    llvm::DIScope* fContext = file;

    // For now, just create a fake subroutine type -- it should be ok
    llvm::DISubroutineType* diFunType = createDiFunType(ctx, fun->type);

    StringRef name = Nest_hasProperty(fun, "name") ? Feather_getName(fun) : StringRef("anonymous");
    llvm::StringRef nameLLVM = toLlvm(name);
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

void DebugInfo::emitParamVar(
        GlobalContext& ctx, Node* param, int idx, llvm::Value* value, llvm::BasicBlock* where) {
    Location loc = param->location;

    ASSERT(!lexicalBlockStack_.empty());
    auto scope = lexicalBlockStack_.back();
    auto file = getOrCreateFile(loc);

    auto diType = createDiType(ctx, param->type);
    auto diVar = diBuilder_.createParameterVariable(
            scope, toLlvm(Feather_getName(param)), idx, file, loc.start.line, diType);

    diBuilder_.insertDeclare(
            value, diVar, diBuilder_.createExpression(), getDebugLoc(loc, scope), where);
}

void DebugInfo::emitLocalVar(GlobalContext& ctx, Node* var, llvm::AllocaInst* llvmAlloca) {
    Location loc = var->location;

    ASSERT(!lexicalBlockStack_.empty());
    auto scope = lexicalBlockStack_.back();
    auto file = getOrCreateFile(loc);

    auto diType = createDiType(ctx, var->type);
    auto diVar = diBuilder_.createAutoVariable(
            scope, toLlvm(Feather_getName(var)), file, loc.start.line, diType);

    diBuilder_.insertDeclare(llvmAlloca, diVar, diBuilder_.createExpression(),
            getDebugLoc(loc, scope), llvmAlloca->getParent());
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

llvm::DISubroutineType* DebugInfo::createDiFunType(GlobalContext& ctx, Type type) {
    llvm::SmallVector<llvm::Metadata*, 8> funTypes;
    for (unsigned i = 0; i < type.type_->numSubtypes; i++)
        funTypes.push_back(createDiType(ctx, type.type_->subTypes[i]));
    return diBuilder_.createSubroutineType(diBuilder_.getOrCreateTypeArray(funTypes));
}

llvm::DIType* DebugInfo::createDiStructType(GlobalContext& ctx, Type type) {
    // Check the cache first
    auto it = typesMap_.find(type);
    if (it != typesMap_.end())
        return it->second;

    // First, check for primitive types
    llvm::Type* t = getLLVMType(type, ctx);
    if (!t)
        return nullptr;
    if (t->isIntegerTy()) {
        auto encoding = t->getPrimitiveSizeInBits() == 1 ? llvm::dwarf::DW_ATE_boolean
                                                         : llvm::dwarf::DW_ATE_signed;
        auto numBits = (t->getPrimitiveSizeInBits() + 7) / 8 * 8;
        return diBuilder_.createBasicType(type.description(), numBits, encoding);
    } else if (t->isFloatTy())
        return diBuilder_.createBasicType(
                "float", t->getPrimitiveSizeInBits(), llvm::dwarf::DW_ATE_float);
    else if (t->isDoubleTy())
        return diBuilder_.createBasicType(
                "double", t->getPrimitiveSizeInBits(), llvm::dwarf::DW_ATE_float);
    else if (t->isFloatingPointTy())
        return diBuilder_.createBasicType(
                "double", t->getPrimitiveSizeInBits(), llvm::dwarf::DW_ATE_float);

    Node* structDecl = type.referredNode();
    ASSERT(structDecl);
    Location loc = structDecl->location;

    llvm::DIFile* file = getOrCreateFile(loc);
    const auto& dataLayout = ctx.llvmModule_.getDataLayout();
    uint64_t sizeInBits = dataLayout.getTypeAllocSizeInBits(t);
    uint32_t alignInBits = dataLayout.getPrefTypeAlignment(t);
    llvm::DINode::DIFlags flags = llvm::DINode::FlagZero;

    // Get the translation name for this
    llvm::StringRef uniqueId = "";
    const Nest_StringRef* nativeName = Nest_getPropertyString(structDecl, propNativeName);
    if (nativeName)
        uniqueId = toLlvm(*nativeName);
    else {
        const Nest_StringRef* description = Nest_getPropertyString(structDecl, propDescription);
        if (description)
            uniqueId = toLlvm(*description);
    }
    llvm::StringRef name = uniqueId;
    if (name.empty())
        name = toLlvm(Feather_getName(structDecl));

    // First create the struct type, with no members
    auto res = diBuilder_.createStructType(file, name, file, loc.start.line, sizeInBits,
            alignInBits, flags, nullptr, llvm::DINodeArray(), 0, nullptr, uniqueId);

    // Add the result to the cache before we descend for the children
    // This allows us to translate recursive structures
    typesMap_[type] = res;

    // Now create the members in the scope of the struct type
    ASSERT(t->isStructTy());
    // NOLINTNEXTLINE
    auto structLayout = dataLayout.getStructLayout((llvm::StructType*)t);
    ASSERT(structLayout);
    llvm::SmallVector<llvm::Metadata*, 8> elements;
    int idx = 0;
    for (auto field : structDecl->children) {
        Location fLoc = field->location;
        auto fieldName = toLlvm(Feather_getName(field));
        auto llvmType = getLLVMType(field->type, ctx);
        if (!llvmType) {
            ++idx;
            continue;
        }
        auto memberType = createDiType(ctx, field->type);
        auto member = diBuilder_.createMemberType(res, fieldName, file, fLoc.start.line,
                dataLayout.getTypeAllocSizeInBits(llvmType),
                dataLayout.getPrefTypeAlignment(llvmType),
                structLayout->getElementOffsetInBits(idx++), flags, memberType);
        elements.push_back(member);
    }
    // Add the members to the struct type
    diBuilder_.replaceArrays(res, diBuilder_.getOrCreateArray(elements));

    return res;
}

llvm::DIType* DebugInfo::createDiType(GlobalContext& ctx, Type type) {
    // For datatypes, type caching is implemented in createDiStructType
    if (type.numReferences() == 0 && type.kind() == Feather_getDataTypeKind())
        return createDiStructType(ctx, type);

    // Check the cache first
    auto it = typesMap_.find(type);
    if (it != typesMap_.end())
        return it->second;

    llvm::Type* t = getLLVMType(type, ctx);
    const auto& dataLayout = ctx.llvmModule_.getDataLayout();

    llvm::DIType* res = nullptr;
    if (type.numReferences() > 0) {
        // Pointer type (Datatype & category type)
        int sizeInBits = dataLayout.getTypeAllocSizeInBits(t);
        auto baseType = createDiType(ctx, removeCatOrRef(TypeWithStorage(type)));
        res = diBuilder_.createPointerType(baseType, sizeInBits);
    } else if (type.kind() == Feather_getArrayTypeKind()) {
        auto baseType = createDiType(ctx, Feather_baseType(type));
        auto numElements = (uint64_t)Feather_getArraySize(type);
        uint32_t alignInBits = dataLayout.getPrefTypeAlignment(t);
        llvm::DINodeArray subscripts;
        return diBuilder_.createArrayType(numElements, alignInBits, baseType, subscripts);

    } else if (type.kind() == Feather_getFunctionTypeKind()) {
        res = createDiFunType(ctx, type);
    } else /*if ( type.kind() == Feather_getVoidTypeKind() )*/ {
        return diBuilder_.createBasicType("void", 0, llvm::dwarf::DW_ATE_address);
    }

    // Add the type to the cache and return it
    typesMap_[type] = res;
    return res;
}
