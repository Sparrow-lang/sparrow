#include <StdInc.h>
#include "TrType.h"
#include "TrTopLevel.h"
#include "Module.h"
#include "GlobalContext.h"

#include "Nest/Utils/Diagnostic.hpp"
#include "Nest/Utils/cppif/StringRef.hpp"
#include "Nest/Utils/cppif/Type.hpp"
#include "Nest/Api/Type.h"
#include "Nest/Api/Node.h"

#include "Feather/Api/Feather.h"
#include "Feather/Utils/FeatherUtils.hpp"
#include "Feather/Utils/cppif/FeatherTypes.hpp"

#include <algorithm>

using namespace LLVMB;
using namespace LLVMB::Tr;
using namespace Nest;
using namespace Feather;

namespace {
llvm::Type* transformVoid(VoidType /*type*/, GlobalContext& ctx) {
    return llvm::Type::getVoidTy(ctx.targetBackend_.llvmContext());
}

llvm::Type* transformDataType(DataType type, GlobalContext& ctx) {
    // Call the translation method for the class declaration
    auto cls = type.referredNode();
    ASSERT(cls);
    // TODO (backend): Sometimes we can generate only opaque structs. No need for fields.
    llvm::Type* t = Tr::translateClass(cls, ctx);
    for (size_t i = 0; i < type.numReferences(); ++i)
        t = llvm::PointerType::get(t, 0);
    return t;
}

llvm::Type* transformPtrType(PtrType type, GlobalContext& ctx) {
    // Translate T ptr, T const ptr, T mut ptr the same way: a simple pointer
    auto base = type.base();
    base = Feather::removeCategoryIfPresent(base);
    return llvm::PointerType::get(getLLVMType(base, ctx), 0);
}

llvm::Type* transformConstType(ConstType type, GlobalContext& ctx) {
    return llvm::PointerType::get(getLLVMType(type.base(), ctx), 0);
}

llvm::Type* transformMutableType(MutableType type, GlobalContext& ctx) {
    return llvm::PointerType::get(getLLVMType(type.base(), ctx), 0);
}

llvm::Type* transformTempType(TempType type, GlobalContext& ctx) {
    return llvm::PointerType::get(getLLVMType(type.base(), ctx), 0);
}

llvm::Type* transformArrayType(ArrayType type, GlobalContext& ctx) {
    return llvm::ArrayType::get(getLLVMType(type.unitType(), ctx), type.count());
}

llvm::Type* transformFunctionType(FunctionType type, int ignoreArg, GlobalContext& ctx) {
    vector<llvm::Type*> llvmParamTypes;
    llvmParamTypes.reserve(type.numParams() + 1);
    llvm::Type* resultType = Tr::getLLVMType(type.result(), ctx);
    for (size_t i = 0; i < type.numParams(); ++i) {
        if (int(i) == ignoreArg)
            continue;
        Type t = type[i];
        llvmParamTypes.push_back(Tr::getLLVMType(t, ctx));
    }
    return llvm::FunctionType::get(resultType, llvmParamTypes, false);
}
} // namespace

llvm::Type* Tr::getLLVMType(Type type, GlobalContext& ctx) {
    if (!type)
        REP_INTERNAL(NOLOC, "Invalid type to translate to LLVM");
    ASSERT(type);
    if (!ctx.targetBackend_.isCt() && !type.canBeUsedAtRt())
        REP_INTERNAL(NOLOC, "Cannot use CT-only type at run-time (%1%)") % type.description();

    // First check or cache of translated types; if we don't have a value there, make sure to set it
    llvm::Type*& llvmType = ctx.targetBackend_.translatedTypes_[type];
    if (llvmType)
        return llvmType;

    if (type.kind() == typeKindVoid)
        llvmType = transformVoid(VoidType(type), ctx);
    else if (type.kind() == typeKindData)
        llvmType = transformDataType(DataType(type), ctx);
    else if (type.kind() == typeKindPtr)
        llvmType = transformPtrType(PtrType(type), ctx);
    else if (type.kind() == typeKindConst)
        llvmType = transformConstType(ConstType(type), ctx);
    else if (type.kind() == typeKindMutable)
        llvmType = transformMutableType(MutableType(type), ctx);
    else if (type.kind() == typeKindTemp)
        llvmType = transformTempType(TempType(type), ctx);
    else if (type.kind() == typeKindArray)
        llvmType = transformArrayType(ArrayType(type), ctx);
    else if (type.kind() == typeKindFunction)
        llvmType = transformFunctionType(FunctionType(type), -1, ctx);
    else {
        REP_INTERNAL(NOLOC, "Don't know how to translate type '%1%'") % type;
        return nullptr;
    }

    return llvmType;
}

llvm::Type* Tr::getNativeLLVMType(
        const Location& loc, StringRef nativeName, llvm::LLVMContext& llvmContext) {
    if (nativeName.size() > 1 && islower(nativeName.begin[0])) {
        if (nativeName == "double")
            return llvm::Type::getDoubleTy(llvmContext);
        else if (nativeName == "float")
            return llvm::Type::getFloatTy(llvmContext);
        else if (nativeName.size() > 1 &&
                 (nativeName.begin[0] == 'i' || nativeName.begin[0] == 'u')) {
            try {
                auto noBits = atoi(nativeName.begin + 1);
                return llvm::IntegerType::get(llvmContext, noBits);
            } catch (...) {
            }
        }

        REP_ERROR(loc, "Unknown native type name: %1%") % nativeName;
    }
    return nullptr;
}

llvm::Type* Tr::getLLVMFunctionType(Node* funDecl, int ignoreArg, GlobalContext& ctx) {
    ASSERT(funDecl);
    Type t = funDecl->type;
    ASSERT(t && t.kind() == typeKindFunction);
    return transformFunctionType(FunctionType(t), ignoreArg, ctx);
}
