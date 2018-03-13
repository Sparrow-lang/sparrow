#include <StdInc.h>
#include "TrType.h"
#include "TrTopLevel.h"
#include "Module.h"
#include "GlobalContext.h"

#include "Nest/Utils/Diagnostic.hpp"
#include "Nest/Utils/StringRef.hpp"
#include "Nest/Api/TypeRef.h"
#include "Nest/Api/Type.h"
#include "Nest/Api/Node.h"

#include "Feather/Api/Feather.h"
#include "Feather/Utils/FeatherUtils.hpp"

#include <boost/bind.hpp>
#include <algorithm>

using namespace LLVMB;
using namespace LLVMB::Tr;
using namespace Nest;
using namespace Feather;

namespace {
llvm::Type* transformVoid(TypeRef /*type*/, GlobalContext& ctx) {
    return llvm::Type::getVoidTy(ctx.targetBackend_.llvmContext());
}

llvm::Type* transformDataType(TypeRef type, GlobalContext& ctx) {
    // Call the translation method for the class declaration
    auto cls = Feather_classDecl(type);
    ASSERT(cls);
    // TODO (backend): Sometimes we can generate only opaque structs. No need for fields.
    llvm::Type* t = Tr::translateClass(cls, ctx);
    for (size_t i = 0; i < type->numReferences; ++i)
        t = llvm::PointerType::get(t, 0);
    return t;
}

llvm::Type* transformLValueType(TypeRef type, GlobalContext& ctx) {
    return llvm::PointerType::get(getLLVMType(Feather_baseType(type), ctx), 0);
}

llvm::Type* transformArrayType(TypeRef type, GlobalContext& ctx) {
    return llvm::ArrayType::get(
            getLLVMType(Feather_baseType(type), ctx), Feather_getArraySize(type));
}

llvm::Type* transformFunctionType(TypeRef type, int ignoreArg, GlobalContext& ctx) {
    vector<llvm::Type*> llvmParamTypes;
    llvmParamTypes.reserve(Feather_numFunParameters(type) + 1);
    llvm::Type* resultType = Tr::getLLVMType(Feather_getFunResultType(type), ctx);
    for (size_t i = 0; i < Feather_numFunParameters(type); ++i) {
        if (int(i) == ignoreArg)
            continue;
        TypeRef t = Feather_getFunParameter(type, i);
        llvmParamTypes.push_back(Tr::getLLVMType(t, ctx));
    }
    return llvm::FunctionType::get(resultType, llvmParamTypes, false);
}
} // namespace

llvm::Type* Tr::getLLVMType(TypeRef type, GlobalContext& ctx) {
    if (!type)
        REP_INTERNAL(NOLOC, "Invalid type to translate to LLVM");
    ASSERT(type);
    if (ctx.targetBackend_.isCt() && type->mode == modeRt)
        REP_INTERNAL(NOLOC, "Cannot use non-CT type at CT (%1%)") % type->description;
    if (!ctx.targetBackend_.isCt() && !type->canBeUsedAtRt)
        REP_INTERNAL(NOLOC, "Cannot use CT-only type at run-time (%1%)") % type->description;

    // First check or cache of translated types; if we don't have a value there, make sure to set it
    llvm::Type*& llvmType = ctx.targetBackend_.translatedTypes_[type];
    if (llvmType)
        return llvmType;

    if (type->typeKind == typeKindVoid)
        llvmType = transformVoid(type, ctx);
    else if (type->typeKind == typeKindData)
        llvmType = transformDataType(type, ctx);
    else if (type->typeKind == typeKindLValue)
        llvmType = transformLValueType(type, ctx);
    else if (type->typeKind == typeKindArray)
        llvmType = transformArrayType(type, ctx);
    else if (type->typeKind == typeKindFunction)
        llvmType = transformFunctionType(type, -1, ctx);
    else {
        REP_INTERNAL(NOLOC, "Don't know how to translate type '%1%'") % type;
        return nullptr;
    }

    return llvmType;
}

llvm::Type* Tr::getNativeLLVMType(
        const Location& loc, StringRef nativeName, llvm::LLVMContext& llvmContext) {
    if (size(nativeName) > 1 && islower(nativeName.begin[0])) {
        if (nativeName == "double")
            return llvm::Type::getDoubleTy(llvmContext);
        else if (nativeName == "float")
            return llvm::Type::getFloatTy(llvmContext);
        else if (size(nativeName) > 1 &&
                 (nativeName.begin[0] == 'i' || nativeName.begin[0] == 'u')) {
            try {
                auto noBits = boost::lexical_cast<int>(nativeName.begin + 1);
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
    TypeRef t = funDecl->type;
    ASSERT(t && t->typeKind == typeKindFunction);
    return transformFunctionType(t, ignoreArg, ctx);
}
