#include <StdInc.h>
#include "DataLayoutHelper.h"
#include <Tr/TrType.h>

#include <Feather/Nodes/Decls/Class.h>
#include <Feather/Nodes/Decls/Var.h>
#include <Feather/Nodes/Properties.h>
#include <Feather/Util/Decl.h>
#include <Feather/FeatherTypes.h>

#include <Nest/Intermediate/Type.h>
#include <Nest/CompilerSettings.h>
#include <Nest/Compiler.h>

using namespace LLVMB;
using namespace Nest;
using namespace Feather;

namespace
{
    llvm::Type* getLLVMTypeForSize(TypeRef type, llvm::LLVMContext& llvmContext)
    {
        // If the number of references is greater than zero, just return an aribitrary pointer type
        if ( type->numReferences > 0 )
            return llvm::PointerType::get(llvm::IntegerType::get(llvmContext, 32), 0);

        // Check array types
        if ( type->typeKind == typeKindArray )
        {
            return llvm::ArrayType::get(getLLVMTypeForSize(baseType(type), llvmContext), getArraySize(type));
        }

        if ( !type->hasStorage )
            REP_ERROR(NOLOC, "Cannot compute size of a type which has no storage: %1%") % type;

        Node* clsDecl = type->referredNode;
        ASSERT(clsDecl && clsDecl->nodeKind == nkFeatherDeclClass);
        if ( !clsDecl->type )
            REP_INTERNAL(clsDecl->location, "Class %1% doesn't have type computed, while computing its size") % getName((DynNode*) clsDecl);

        // Check if this is a standard/native type
        const string* nativeName = Nest::getPropertyString(clsDecl, propNativeName);
        if ( nativeName )
        {
            llvm::Type* t = Tr::getNativeLLVMType(clsDecl->location, *nativeName, llvmContext);
            if ( t )
                return t;
        }

        // Create the type, and set it as a property (don't add any subtypes yet to avoid endless loops)
        const string* description = Nest::getPropertyString(clsDecl, propDescription);
        llvm::StructType* t = llvm::StructType::create(llvmContext, description ? *description : getName((DynNode*) clsDecl));

        // Now add the subtypes
        vector<llvm::Type*> fieldTypes;
        fieldTypes.reserve(clsDecl->children.size());
        for ( auto field: clsDecl->children )
        {
            fieldTypes.push_back(getLLVMTypeForSize(field->type, llvmContext));
        }
        t->setBody(fieldTypes);
        return t;
    }
}


DataLayoutHelper::DataLayoutHelper()
    : llvmContext_(new llvm::LLVMContext())
	, llvmModule_(new llvm::Module("Module for computing data layout", *llvmContext_))
{
    Nest::CompilerSettings& s = Nest::theCompiler().settings();

    llvmModule_->setDataLayout(s.dataLayout_);
    llvmModule_->setTargetTriple(s.targetTriple_);
}

DataLayoutHelper::~DataLayoutHelper()
{
    delete llvmModule_;
    delete llvmContext_;
}

size_t DataLayoutHelper::getSizeOf(Nest::TypeRef type)
{
    // Special case for "Type" type
    if ( 0 == strcmp(type->description, "Type/ct") )
        return sizeof(const char*);

    // Check if we already computed this
    auto it = sizesOfTypes_.find(type);
    if ( it != sizesOfTypes_.end() )
        return it->second;

    // Start computing it now -- make sure we store the result in the map
    size_t& size = sizesOfTypes_[type];

    llvm::Type* llvmType = getLLVMTypeForSize(type, *llvmContext_);
    auto dataLayout = llvmModule_->getDataLayout();
    size = dataLayout->getTypeAllocSize(llvmType);
    return size;
}

size_t DataLayoutHelper::getAlignOf(Nest::TypeRef type)
{
#ifdef _MSC_VER
    #define ALIGNOF(x) __alignof(x)
#else
    #define ALIGNOF(x) alignof(x)
#endif

    // Special case for "Type" type
    if ( 0 == strcmp(type->description, "Type/ct") )
        return ALIGNOF(const char*);

    // Check if we already computed this
    auto it = alignmentsOfTypes_.find(type);
    if ( it != alignmentsOfTypes_.end() )
        return it->second;

    // Start computing it now -- make sure we store the result in the map
    size_t& align = alignmentsOfTypes_[type];
    
    llvm::Type* llvmType = getLLVMTypeForSize(type, *llvmContext_);
    auto dataLayout = llvmModule_->getDataLayout();
    align = dataLayout->getPrefTypeAlignment(llvmType) / 8;
    return align;
}
