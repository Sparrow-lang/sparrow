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

VoidType VoidType::changeMode(Nest::EvalMode mode, Nest::Location /*loc*/) const {
    return get(mode); // No checks here. Always succeeds
}

DataType::DataType(Nest::TypeRef type)
    : TypeWithStorage(type) {
    if (type && type->typeKind != Feather_getDataTypeKind())
        REP_INTERNAL(NOLOC, "DataType constructed with other type kind (%1%)") % type;
}

DataType DataType::get(Nest::NodeHandle decl, Nest::EvalMode mode) {
    return {Feather_getDataType(decl, 0, mode)};
}

PtrType::PtrType(Nest::TypeRef type)
    : TypeWithStorage(type) {
    if (type && type->typeKind != Feather_getPtrTypeKind())
        REP_INTERNAL(NOLOC, "PtrType constructed with other type kind (%1%)") % type;
}

PtrType PtrType::get(TypeWithStorage base, Nest::Location loc) {
    if (!base)
        REP_INTERNAL(loc, "Null type given as base to ptr type");
    return {Feather_getPtrType(base)};
}

ConstType::ConstType(Nest::TypeRef type)
    : TypeWithStorage(type) {
    if (type && type->typeKind != Feather_getConstTypeKind())
        REP_INTERNAL(NOLOC, "ConstType constructed with other type kind (%1%)") % type;
}

ConstType ConstType::get(TypeWithStorage base, Nest::Location loc) {
    if (!base)
        REP_INTERNAL(loc, "Null type given as base to const type");
    int baseKind = base.kind();
    if (baseKind == typeKindConst)
        return {base};
    else if (baseKind == typeKindMutable || baseKind == typeKindTemp)
        REP_ERROR(loc, "Cannot construct a const type based on %1%") % base;
    return {Feather_getConstType(base)};
}

TypeWithStorage ConstType::toRef() const {
    return getDataTypeWithPtr(referredNode(), numReferences(), mode());
}

MutableType::MutableType(Nest::TypeRef type)
    : TypeWithStorage(type) {
    if (type && type->typeKind != Feather_getMutableTypeKind())
        REP_INTERNAL(NOLOC, "MutableType constructed with other type kind (%1%)") % type;
}

MutableType MutableType::get(TypeWithStorage base, Nest::Location loc) {
    if (!base)
        REP_INTERNAL(loc, "Null type given as base to mutable type");
    int baseKind = base.kind();
    if (baseKind == typeKindMutable)
        return {base};
    else if (baseKind == typeKindConst || baseKind == typeKindTemp)
        REP_ERROR(loc, "Cannot construct a mutable type based on %1%") % base;
    return {Feather_getMutableType(base)};
}

TypeWithStorage MutableType::toRef() const {
    return getDataTypeWithPtr(referredNode(), numReferences(), mode());
}

TempType::TempType(Nest::TypeRef type)
    : TypeWithStorage(type) {
    if (type && type->typeKind != Feather_getTempTypeKind())
        REP_INTERNAL(NOLOC, "TempType constructed with other type kind (%1%)") % type;
}

TempType TempType::get(TypeWithStorage base, Nest::Location loc) {
    if (!base)
        REP_INTERNAL(loc, "Null type given as base to const type");
    int baseKind = base.kind();
    if (baseKind == typeKindTemp)
        return {base};
    else if (baseKind == typeKindConst || baseKind == typeKindMutable)
        REP_ERROR(loc, "Cannot construct a tmp type based on %1%") % base;
    return {Feather_getTempType(base)};
}

TypeWithStorage TempType::toRef() const {
    return getDataTypeWithPtr(referredNode(), numReferences(), mode());
}

ArrayType::ArrayType(Nest::TypeRef type)
    : TypeWithStorage(type) {
    if (type && type->typeKind != Feather_getArrayTypeKind())
        REP_INTERNAL(NOLOC, "ArrayType constructed with other type kind (%1%)") % type;
}

ArrayType ArrayType::get(TypeWithStorage unitType, int count) {
    return {Feather_getArrayType(unitType, count)};
}

FunctionType::FunctionType(Nest::TypeRef type)
    : TypeWithStorage(type) {
    if (type && type->typeKind != Feather_getFunctionTypeKind())
        REP_INTERNAL(NOLOC, "FunctionType constructed with other type kind (%1%)") % type;
}

FunctionType FunctionType::get(
        Nest::TypeRef* resultTypeAndParams, int numTypes, Nest::EvalMode mode) {
    return {Feather_getFunctionType(resultTypeAndParams, numTypes, mode)};
}

bool isDataLikeType(Type type) {
    int typeKind = type.kind();
    return typeKind == typeKindData || typeKind == typeKindPtr || typeKind == typeKindConst ||
           typeKind == typeKindMutable || typeKind == typeKindTemp;
}

bool isCategoryType(Type type) {
    int typeKind = type.kind();
    return typeKind == typeKindConst || typeKind == typeKindMutable || typeKind == typeKindTemp;
}

TypeWithStorage getDataTypeWithPtr(Nest::NodeHandle decl, int numReferences, Nest::EvalMode mode) {
    TypeWithStorage res = DataType::get(decl, mode);
    for (int i = 0; i < numReferences; i++)
        res = PtrType::get(res);
    return res;
}

TypeWithStorage addRef(TypeWithStorage type) {
    if (!type)
        REP_INTERNAL(NOLOC, "Null type passed to addRef");

    int typeKind = type.kind();
    if (typeKind == typeKindData)
        return getDataTypeWithPtr(type.referredNode(), type.numReferences() + 1, type.mode());
    else if (typeKind == typeKindPtr)
        return PtrType::get(type);
    else if (typeKind == typeKindConst)
        return ConstType::get(addRef(ConstType(type).base()));
    else if (typeKind == typeKindMutable)
        return MutableType::get(addRef(MutableType(type).base()));
    else if (typeKind == typeKindTemp)
        return TempType::get(addRef(TempType(type).base()));

    REP_INTERNAL(NOLOC, "Invalid type given when adding reference (%1%)") % type;
    return {};
}
TypeWithStorage removeRef(TypeWithStorage type) {
    if (!type)
        REP_INTERNAL(NOLOC, "Null type passed to removeRef");

    int typeKind = type.kind();
    if (typeKind == typeKindData) {
        if (type.numReferences() == 0)
            REP_INTERNAL(NOLOC, "Cannot remove reference from %1%") % type;
        return getDataTypeWithPtr(type.referredNode(), type.numReferences() - 1, type.mode());
    } else if (typeKind == typeKindPtr)
        return PtrType(type).base();
    else if (typeKind == typeKindConst)
        return ConstType::get(removeRef(ConstType(type).base()));
    else if (typeKind == typeKindMutable)
        return MutableType::get(removeRef(MutableType(type).base()));
    else if (typeKind == typeKindTemp)
        return TempType::get(removeRef(TempType(type).base()));

    REP_INTERNAL(NOLOC, "Invalid type given when removing reference (%1%)") % type;
    return {};
}

TypeWithStorage removeCatOrRef(TypeWithStorage type) {
    if (!type)
        REP_INTERNAL(NOLOC, "Null type passed to removeRef");
    if (type.numReferences() < 1)
        REP_INTERNAL(NOLOC, "Cannot remove reference from type (%1%)") % type;

    int typeKind = type.kind();
    if (typeKind == typeKindData || typeKind == typeKindPtr || typeKind == typeKindConst ||
            typeKind == typeKindMutable || typeKind == typeKindTemp)
        return getDataTypeWithPtr(type.referredNode(), type.numReferences() - 1, type.mode());

    REP_INTERNAL(NOLOC, "Invalid type given when removing reference (%1%)") % type;
    return {};
}

TypeWithStorage removeAllRefs(TypeWithStorage type) {
    if (!type)
        REP_INTERNAL(NOLOC, "Null type passed to removeAllRefs");

    int typeKind = type.kind();
    if (typeKind == typeKindData || typeKind == typeKindPtr || typeKind == typeKindConst || typeKind == typeKindMutable ||
            typeKind == typeKindTemp)
        return DataType::get(type.referredNode(), type.mode());

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

TypeWithStorage changeCat(TypeWithStorage type, int newCatKind) {
    if (!type)
        REP_INTERNAL(NOLOC, "Null type passed to changeCat");
    if (type.numReferences() < 1)
        REP_INTERNAL(NOLOC, "Cannot change cat of non-reference type %1%") % type;

    int baseKind = type.kind();
    if (baseKind != typeKindData && baseKind != typeKindPtr && baseKind != typeKindConst && baseKind != typeKindMutable &&
            baseKind != typeKindTemp)
        REP_INTERNAL(NOLOC, "Invalid type given to change category: %1%") % type;

    TypeWithStorage base =
            getDataTypeWithPtr(type.referredNode(), type.numReferences() - 1, type.mode());

    if (newCatKind == typeKindConst)
        return ConstType::get(base);
    else if (newCatKind == typeKindMutable)
        return MutableType::get(base);
    else if (newCatKind == typeKindTemp)
        return TempType::get(base);

    REP_INTERNAL(NOLOC, "Invalid type kind given when changing category(%1%)") % newCatKind;
    return {};
}

} // namespace Feather
