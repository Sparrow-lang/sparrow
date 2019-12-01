#include <StdInc.h>
#include "DataLayoutHelper.h"
#include <Tr/TrType.h>

#include "Feather/Api/Feather.h"
#include "Feather/Utils/FeatherUtils.hpp"
#include "Feather/Utils/cppif/FeatherTypes.hpp"

#include "Nest/Api/Node.h"
#include "Nest/Utils/cppif/NodeUtils.hpp"
#include "Nest/Utils/cppif/Type.hpp"
#include "Nest/Api/Type.h"
#include "Nest/Utils/CompilerSettings.hpp"
#include "Nest/Api/Compiler.h"
#include "Nest/Utils/Diagnostic.hpp"
#include "Nest/Utils/cppif/StringRef.hpp"
#include "Nest/Utils/NodeUtils.h"

using namespace LLVMB;

namespace {
llvm::Type* getLLVMTypeForSize(Type type, llvm::LLVMContext& llvmContext) {
    // If the number of references is greater than zero, just return an arbitrary pointer type
    if (type.numReferences() > 0)
        return llvm::PointerType::get(llvm::IntegerType::get(llvmContext, 32), 0);

    // Check array types
    if (type.kind() == typeKindArray) {
        Feather::ArrayType arrayType{type};
        return llvm::ArrayType::get(
                getLLVMTypeForSize(arrayType.unitType(), llvmContext), arrayType.count());
    }

    if (!type.hasStorage())
        REP_ERROR_RET(nullptr, NOLOC, "Cannot compute size of a type which has no storage: %1%") %
                type;

    Node* clsDecl = type.referredNode();
    CHECK(NOLOC, clsDecl && clsDecl->nodeKind == nkFeatherDeclClass);

    // Check if this is a standard/native type
    const Nest_StringRef* nativeName = Nest_getPropertyString(clsDecl, propNativeName);
    if (nativeName) {
        llvm::Type* t = Tr::getNativeLLVMType(clsDecl->location, *nativeName, llvmContext);
        if (t)
            return t;
    }

    if (!clsDecl->type)
        REP_INTERNAL(clsDecl->location,
                "Class %1% doesn't have type computed, while computing its size") %
                Feather_getName(clsDecl);

    // Create the type, and set it as a property (don't add any subtypes yet to avoid endless loops)
    const Nest_StringRef* description = Nest_getPropertyString(clsDecl, propDescription);
    StringRef desc = description ? *description : Feather_getName(clsDecl);
    llvm::StringRef descLLVM(desc.begin, desc.size());
    llvm::StructType* t = llvm::StructType::create(llvmContext, descLLVM);

    // Now add the subtypes
    vector<llvm::Type*> fieldTypes;
    fieldTypes.reserve(Nest_nodeArraySize(clsDecl->children));
    for (auto field : clsDecl->children) {
        fieldTypes.push_back(getLLVMTypeForSize(field->type, llvmContext));
    }
    t->setBody(fieldTypes);
    return t;
}
} // namespace

DataLayoutHelper::DataLayoutHelper()
    : llvmContext_(new llvm::LLVMContext())
    , llvmModule_(new llvm::Module("Module for computing data layout", *llvmContext_)) {
    auto& s = *Nest_compilerSettings();

    llvmModule_->setDataLayout(s.dataLayout_);
    llvmModule_->setTargetTriple(s.targetTriple_);
}

DataLayoutHelper::~DataLayoutHelper() = default;

size_t DataLayoutHelper::getSizeOf(Type type) {
    // Special case for "Type" type
    if (0 == strcmp(type.description(), "Type/ct"))
        return sizeof(const char*);

    // Check if we already computed this
    auto it = sizesOfTypes_.find(type);
    if (it != sizesOfTypes_.end())
        return it->second;

    // Start computing it now -- make sure we store the result in the map
    size_t& size = sizesOfTypes_[type];

    llvm::Type* llvmType = getLLVMTypeForSize(type, *llvmContext_);
    const auto& dataLayout = llvmModule_->getDataLayout();
    size = dataLayout.getTypeAllocSize(llvmType);
    return size;
}

size_t DataLayoutHelper::getAlignOf(Type type) {
    // Special case for "Type" type
    if (0 == strcmp(type.description(), "Type/ct"))
        return alignof(const char*);

    // Check if we already computed this
    auto it = alignmentsOfTypes_.find(type);
    if (it != alignmentsOfTypes_.end())
        return it->second;

    // Start computing it now -- make sure we store the result in the map
    size_t& align = alignmentsOfTypes_[type];

    llvm::Type* llvmType = getLLVMTypeForSize(type, *llvmContext_);
    const auto& dataLayout = llvmModule_->getDataLayout();
    align = dataLayout.getPrefTypeAlignment(llvmType) / 8;
    return align;
}
