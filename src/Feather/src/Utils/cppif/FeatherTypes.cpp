#include "Feather/src/StdInc.h"
#include "Feather/Utils/cppif/FeatherTypes.hpp"
#include "Feather/Api/Feather.h"
#include "Feather/Utils/FeatherUtils.h"
#include "Nest/Utils/Diagnostic.hpp"

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
        REP_INTERNAL(NOLOC, "DataType constructed with other type kind (%1%") % type;
}

DataType DataType::get(Nest::NodeHandle decl, int numReferences, Nest::EvalMode mode) {
    return DataType(Feather_getDataType(decl, numReferences, mode));
}

LValueType::LValueType(Nest::TypeRef type)
    : TypeWithStorage(type) {
    if (type && type->typeKind != Feather_getLValueTypeKind())
        REP_INTERNAL(NOLOC, "LValueType constructed with other type kind (%1%") % type;
}

DataType LValueType::toRef() const {
    return DataType::get(referredNode(), numReferences(), mode());
}

LValueType LValueType::get(TypeWithStorage base) {
    if (!base)
        REP_INTERNAL(NOLOC, "Null type given as base to LValue type");
    if (base.kind() == typeKindLValue)
        return LValueType(base);
    return LValueType(Feather_getLValueType(base));
}

ArrayType::ArrayType(Nest::TypeRef type)
    : TypeWithStorage(type) {
    if (type && type->typeKind != Feather_getArrayTypeKind())
        REP_INTERNAL(NOLOC, "ArrayType constructed with other type kind (%1%") % type;
}

ArrayType ArrayType::get(TypeWithStorage unitType, int count) {
    return ArrayType(Feather_getArrayType(unitType, count));
}

FunctionType::FunctionType(Nest::TypeRef type)
    : TypeWithStorage(type) {
    if (type && type->typeKind != Feather_getFunctionTypeKind())
        REP_INTERNAL(NOLOC, "FunctionType constructed with other type kind (%1%") % type;
}

FunctionType FunctionType::get(
        Nest::TypeRef* resultTypeAndParams, int numTypes, Nest::EvalMode mode) {
    return FunctionType(Feather_getFunctionType(resultTypeAndParams, numTypes, mode));
}

DataType addRef(TypeWithStorage type) {
    if (!type)
        REP_INTERNAL(NOLOC, "Null type passed to addRef");
    if (type.kind() != typeKindData && type.kind() != typeKindLValue)
        REP_INTERNAL(NOLOC, "Invalid type given when adding reference (%1%)") % type;
    return DataType::get(type.referredNode(), type.numReferences() + 1, type.mode());
}
DataType removeRef(TypeWithStorage type) {
    if (!type)
        REP_INTERNAL(NOLOC, "Null type passed to removeRef");
    if (type.numReferences() < 1)
        REP_INTERNAL(NOLOC, "Cannot remove reference from type (%1%)") % type;
    if (type.kind() != typeKindData && type.kind() != typeKindLValue)
        REP_INTERNAL(NOLOC, "Invalid type given when removing reference (%1%)") % type;
    return DataType::get(type.referredNode(), type.numReferences() - 1, type.mode());
}
DataType removeAllRefs(TypeWithStorage type) {
    if (!type)
        REP_INTERNAL(NOLOC, "Null type passed to removeAllRefs");
    if (type.kind() != typeKindData && type.kind() != typeKindLValue)
        REP_INTERNAL(NOLOC, "Invalid type given when removing all references (%1%)") % type;
    return DataType::get(type.referredNode(), 0, type.mode());
}

Type removeLValueIfPresent(Type type) {
    if (type.kind() == typeKindLValue)
        return LValueType(type.type_).base();
    else
        return type;
}

Type lvalueToRefIfPresent(Type type) {
    if (type.kind() == typeKindLValue)
        return LValueType(type.type_).toRef();
    else
        return type;
}

} // namespace Feather
