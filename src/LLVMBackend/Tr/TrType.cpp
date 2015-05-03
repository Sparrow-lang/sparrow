#include <StdInc.h>
#include "TrType.h"
#include "TrTopLevel.h"
#include "Module.h"

#include <Nest/Common/Diagnostic.h>

#include <Feather/Type/Void.h>
#include <Feather/Type/DataType.h>
#include <Feather/Type/LValueType.h>
#include <Feather/Type/ArrayType.h>
#include <Feather/Type/FunctionType.h>
#include <Feather/Nodes/Decls/Function.h>

#include <boost/bind.hpp>
#include <algorithm>

using namespace LLVMB;
using namespace LLVMB::Tr;
using namespace Nest;
using namespace Feather;

namespace
{
    llvm::Type* transform(Void& /*type*/, Module& module)
    {
        return llvm::Type::getVoidTy(module.llvmContext());
    }

    llvm::Type* transform(DataType& type, Module& module)
    {
        // Call the translation method for the class declaration
        auto cls = type.classDecl();
        ASSERT(cls);
        llvm::Type* t = Tr::translateClass(cls, module);
        for ( size_t i=0; i<type.noReferences(); ++i )
            t = llvm::PointerType::get(t, 0);
        return t;
    }

    llvm::Type* transform(LValueType& type, Module& module)
    {
        llvm::Type* t = llvm::PointerType::get(getLLVMType(type.baseType(), module), 0);
        return t;
    }

    llvm::Type* transform(ArrayType& type, Module& module)
    {
        llvm::Type* t = llvm::ArrayType::get(getLLVMType(type.unitType(), module), type.count());
        return t;
    }

    llvm::Type* transform(FunctionType& type, int ignoreArg, Module& module)
    {
        vector<llvm::Type*> llvmParamTypes;
        llvmParamTypes.reserve(type.noParameters()+1);
        llvm::Type* resultType = Tr::getLLVMType(type.resultType(), module);
        for ( size_t i=0; i<type.noParameters(); ++i )
        {
            if ( int(i) == ignoreArg )
                continue;
            Type* t = type.getParameter(i);
            llvmParamTypes.push_back(Tr::getLLVMType(t, module));
        }
        return llvm::FunctionType::get(resultType, llvmParamTypes, false);
    }
}

llvm::Type* Tr::getLLVMType(Type* type, Module& module)
{
    if ( !type )
        REP_INTERNAL(NOLOC, "Invalid type to translate to LLVM");
    if ( module.isCt() && type->mode() == modeRt )
        REP_INTERNAL(NOLOC, "Cannot use non-CT type at CT (%1%)") % type->toString();
    if ( !module.isCt() && !type->canBeUsedAtRt() )
        REP_INTERNAL(NOLOC, "Cannot use CT-only type at run-time (%1%)") % type->toString();

    // First check or cache of translated types; if we don't have a value there, make sure to set it
    llvm::Type*& llvmType = module.translatedTypes_[type];
    if ( llvmType )
        return llvmType;

    switch ( type->typeId() )
    {
    case Type::typeVoid:
        llvmType = transform(static_cast<Void&>(*type), module);
        break;
    case Type::typeData:
        llvmType = transform(static_cast<DataType&>(*type), module);
        break;
    case Type::typeLValue:
        llvmType = transform(static_cast<LValueType&>(*type), module);
        break;
    case Type::typeArray:
        llvmType = transform(static_cast<ArrayType&>(*type), module);
        break;
    case Type::typeFunction:
        llvmType = transform(static_cast<FunctionType&>(*type), -1, module);
        break;
    default:
        {
            REP_ERROR(NOLOC, "Don't know how to translate type '%1%'") % type;
            return nullptr;
        }
    }

    return llvmType;
}

llvm::Type* Tr::getNativeLLVMType(const Nest::Location& loc, const string& nativeName, llvm::LLVMContext& llvmContext)
{
    if ( nativeName.size() > 1 && islower(nativeName[0]) )
    {
        llvm::Type* t = nullptr;

        if ( nativeName == "double" )
            t = llvm::Type::getDoubleTy(llvmContext);
        else if ( nativeName == "float" )
            t = llvm::Type::getFloatTy(llvmContext);
        else if ( nativeName.size() > 1 && (nativeName[0] == 'i' || nativeName[0] == 'u') )
        {
            try
            {
                int noBits = boost::lexical_cast<int>(nativeName.substr(1));
                t = llvm::IntegerType::get(llvmContext, noBits);
            }
            catch (...)
            {
            }
        }

        if ( !t )
            REP_ERROR(loc, "Unknown native type name: %1%") % nativeName;
        return t;
    }
    return nullptr;
}

llvm::Type* Tr::getLLVMFunctionType(Feather::Function* funDecl, int ignoreArg, Module& module)
{
    ASSERT(funDecl);
    Type* t = funDecl->type();
    ASSERT(t && t->typeId() == Type::typeFunction);
    return transform(static_cast<FunctionType&>(*t), ignoreArg, module);
}
