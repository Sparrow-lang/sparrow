#include "Feather/src/StdInc.h"
#include "Feather/Utils/cppif/FeatherTypes.hpp"
#include "Feather/Api/Feather.h"
#include "Feather/Utils/FeatherUtils.h"
#include "Nest/Utils/Diagnostic.hpp"
#include "Nest/Utils/cppif/NodeHandle.hpp"

namespace Feather {

VoidType::VoidType(Nest::TypeRef type)
    : Type(type) {
    ASSERT(!type || type->typeKind == Feather_getVoidTypeKind());
}

VoidType VoidType::get(Nest::EvalMode mode) { return VoidType(Feather_getVoidType(mode)); }

VoidType VoidType::changeMode(Nest::EvalMode mode, Nest::Location /*loc*/) {
    return get(mode); // No checks here. Always succeeds
}

DataType::DataType(Nest::TypeRef type)
    : TypeWithStorage(type) {
    if (type && type->typeKind != Feather_getDataTypeKind())
        REP_INTERNAL(NOLOC, "DataType constructed with other type kind (%1%)") % type;
}

DataType DataType::get(Nest::NodeHandle decl, int numReferences, Nest::EvalMode mode) {
    return DataType(Feather_getDataType(decl, numReferences, mode));
}

ConstType::ConstType(Nest::TypeRef type)
    : TypeWithStorage(type) {
    if (type && type->typeKind != Feather_getConstTypeKind())
        REP_INTERNAL(NOLOC, "ConstType constructed with other type kind (%1%)") % type;
}

ConstType ConstType::get(TypeWithStorage base) {
    if (!base)
        REP_INTERNAL(NOLOC, "Null type given as base to const type");
    int baseKind = base.kind();
    if (baseKind == typeKindConst)
        return ConstType(base);
    else if (baseKind == typeKindMutable || baseKind == typeKindTemp)
        REP_INTERNAL(NOLOC, "Cannot construct a const type based on %1%") % base;
    return ConstType(Feather_getConstType(base));
}

DataType ConstType::toRef() const { return DataType::get(referredNode(), numReferences(), mode()); }

MutableType::MutableType(Nest::TypeRef type)
    : TypeWithStorage(type) {
    if (type && type->typeKind != Feather_getMutableTypeKind())
        REP_INTERNAL(NOLOC, "MutableType constructed with other type kind (%1%)") % type;
}

MutableType MutableType::get(TypeWithStorage base) {
    if (!base)
        REP_INTERNAL(NOLOC, "Null type given as base to const type");
    int baseKind = base.kind();
    if (baseKind == typeKindMutable)
        return MutableType(base);
    else if (baseKind == typeKindConst || baseKind == typeKindTemp)
        REP_INTERNAL(NOLOC, "Cannot construct a mutable type based on %1%") % base;
    return MutableType(Feather_getMutableType(base));
}

DataType MutableType::toRef() const {
    return DataType::get(referredNode(), numReferences(), mode());
}

TempType::TempType(Nest::TypeRef type)
    : TypeWithStorage(type) {
    if (type && type->typeKind != Feather_getTempTypeKind())
        REP_INTERNAL(NOLOC, "TempType constructed with other type kind (%1%)") % type;
}

TempType TempType::get(TypeWithStorage base) {
    if (!base)
        REP_INTERNAL(NOLOC, "Null type given as base to const type");
    int baseKind = base.kind();
    if (baseKind == typeKindTemp)
        return TempType(base);
    else if (baseKind == typeKindConst || baseKind == typeKindMutable)
        REP_INTERNAL(NOLOC, "Cannot construct a tmp type based on %1%") % base;
    return TempType(Feather_getTempType(base));
}

DataType TempType::toRef() const { return DataType::get(referredNode(), numReferences(), mode()); }

ArrayType::ArrayType(Nest::TypeRef type)
    : TypeWithStorage(type) {
    if (type && type->typeKind != Feather_getArrayTypeKind())
        REP_INTERNAL(NOLOC, "ArrayType constructed with other type kind (%1%)") % type;
}

ArrayType ArrayType::get(TypeWithStorage unitType, int count) {
    return ArrayType(Feather_getArrayType(unitType, count));
}

FunctionType::FunctionType(Nest::TypeRef type)
    : TypeWithStorage(type) {
    if (type && type->typeKind != Feather_getFunctionTypeKind())
        REP_INTERNAL(NOLOC, "FunctionType constructed with other type kind (%1%)") % type;
}

FunctionType FunctionType::get(
        Nest::TypeRef* resultTypeAndParams, int numTypes, Nest::EvalMode mode) {
    return FunctionType(Feather_getFunctionType(resultTypeAndParams, numTypes, mode));
}

bool isDataLikeType(Type type) {
    int typeKind = type.kind();
    return typeKind == typeKindData || typeKind == typeKindConst || typeKind == typeKindMutable ||
           typeKind == typeKindTemp;
}

bool isCategoryType(Type type) {
    int typeKind = type.kind();
    return typeKind == typeKindConst || typeKind == typeKindMutable || typeKind == typeKindTemp;
}

TypeWithStorage addRef(TypeWithStorage type) {
    if (!type)
        REP_INTERNAL(NOLOC, "Null type passed to addRef");
    int typeKind = type.kind();
    if (typeKind == typeKindData || typeKind == typeKindConst || typeKind == typeKindMutable ||
            typeKind == typeKindTemp)
        return DataType::get(type.referredNode(), type.numReferences() + 1, type.mode());
    // else if (typeKind == typeKindConst) {
    //     if (ConstType(type).base().kind() == typeKindData)
    //         return ConstType::get(
    //                 DataType::get(type.referredNode(), type.numReferences() + 1, type.mode()));
    // } else if (typeKind == typeKindMutable) {
    //     if (MutableType(type).base().kind() == typeKindData)
    //         return MutableType::get(
    //                 DataType::get(type.referredNode(), type.numReferences() + 1, type.mode()));
    // } else if (typeKind == typeKindTemp) {
    //     if (TempType(type).base().kind() == typeKindData)
    //         return TempType::get(
    //                 DataType::get(type.referredNode(), type.numReferences() + 1, type.mode()));
    // }

    REP_INTERNAL(NOLOC, "Invalid type given when adding reference (%1%)") % type;
    return {};
}
TypeWithStorage removeRef(TypeWithStorage type) {
    if (!type)
        REP_INTERNAL(NOLOC, "Null type passed to removeRef");
    if (type.numReferences() < 1)
        REP_INTERNAL(NOLOC, "Cannot remove reference from type (%1%)") % type;

    int typeKind = type.kind();
    if (typeKind == typeKindData || typeKind == typeKindConst || typeKind == typeKindMutable ||
            typeKind == typeKindTemp)
        return DataType::get(type.referredNode(), type.numReferences() - 1, type.mode());
    // else if (typeKind == typeKindConst) {
    //     if (ConstType(type).base().kind() == typeKindData)
    //         return ConstType::get(
    //                 DataType::get(type.referredNode(), type.numReferences() - 1, type.mode()));
    // } else if (typeKind == typeKindMutable) {
    //     if (MutableType(type).base().kind() == typeKindData)
    //         return MutableType::get(
    //                 DataType::get(type.referredNode(), type.numReferences() - 1, type.mode()));
    // } else if (typeKind == typeKindTemp) {
    //     if (TempType(type).base().kind() == typeKindData)
    //         return TempType::get(
    //                 DataType::get(type.referredNode(), type.numReferences() - 1, type.mode()));
    // }

    REP_INTERNAL(NOLOC, "Invalid type given when removing reference (%1%)") % type;
    return {};
}
TypeWithStorage removeAllRefs(TypeWithStorage type) {
    if (!type)
        REP_INTERNAL(NOLOC, "Null type passed to removeAllRefs");

    int typeKind = type.kind();
    if (typeKind == typeKindData || typeKind == typeKindConst || typeKind == typeKindMutable ||
            typeKind == typeKindTemp)
        return DataType::get(type.referredNode(), 0, type.mode());
    // else if (typeKind == typeKindConst) {
    //     if (ConstType(type).base().kind() == typeKindData)
    //         return ConstType::get(DataType::get(type.referredNode(), 0, type.mode()));
    // } else if (typeKind == typeKindMutable) {
    //     if (MutableType(type).base().kind() == typeKindData)
    //         return MutableType::get(DataType::get(type.referredNode(), 0, type.mode()));
    // } else if (typeKind == typeKindTemp) {
    //     if (TempType(type).base().kind() == typeKindData)
    //         return TempType::get(DataType::get(type.referredNode(), 0, type.mode()));
    // }

    REP_INTERNAL(NOLOC, "Invalid type given when removing all references (%1%)") % type;
    return {};
}

Type removeCategoryIfPresent(Type type) {
    if (type.kind() == typeKindConst)
        return ConstType(type.type_).base();
    else if (type.kind() == typeKindMutable)
        return MutableType(type.type_).base();
    else if (type.kind() == typeKindTemp)
        return TempType(type.type_).base();
    else
        return type;
}
TypeWithStorage removeCategoryIfPresent(TypeWithStorage type) {
    if (type.kind() == typeKindConst)
        return ConstType(type.type_).base();
    else if (type.kind() == typeKindMutable)
        return MutableType(type.type_).base();
    else if (type.kind() == typeKindTemp)
        return TempType(type.type_).base();
    else
        return type;
}

Type categoryToRefIfPresent(Type type) {
    if (type.kind() == typeKindConst)
        return ConstType(type.type_).toRef();
    else if (type.kind() == typeKindMutable)
        return MutableType(type.type_).toRef();
    else if (type.kind() == typeKindTemp)
        return TempType(type.type_).toRef();
    else
        return type;
}

} // namespace Feather
