#include "Feather/src/StdInc.h"
#include "Feather/Utils/cppif/FeatherTypes.hpp"
#include "Feather/Api/Feather.h"
#include "Feather/Utils/FeatherUtils.h"
#include "Nest/Utils/Diagnostic.hpp"
#include "Nest/Utils/cppif/NodeHandle.hpp"
#include "Nest/Utils/Alloc.h"
#include "Nest/Utils/cppif/StringRef.hpp"

namespace Feather {

namespace {

using Nest::TypeRef;

const char* str(const string& s) { return dupString(s.c_str()); }

const char* getVoidDescription(EvalMode mode) {
    switch (mode) {
    case modeCt:
        return "Void/ct";
    case modeRt:
    default:
        return "Void";
    }
}
string getDataTypeDescription(Nest::NodeHandle classDecl, EvalMode mode) {
    string res;
    if (classDecl) {
        auto description = classDecl.getPropertyString(propDescription);
        Nest::StringRef desc =
                description ? *description : Nest::StringRef(Feather_getName(classDecl));
        res += string(desc.begin, desc.end);
    } else
        res += "<no class>";
    if (mode == modeCt)
        res += "/ct";
    return res;
}
string getPtrTypeDescription(TypeRef base) { return string(base->description) + " ptr"; }
string getConstTypeDescription(TypeRef base) { return string(base->description) + " const"; }
string getMutableTypeDescription(TypeRef base) { return string(base->description) + " mut"; }
string getTempTypeDescription(TypeRef base) { return string(base->description) + " tmp"; }
string getArrayTypeDescription(TypeRef unitType, unsigned count) {
    ostringstream oss;
    oss << unitType->description << " A(" << count << ")";
    return oss.str();
}
string Feather_getFunctionTypeDescription(
        TypeRef* resultTypeAndParams, unsigned numTypes, EvalMode mode) {
    ostringstream oss;
    oss << "F(";
    for (unsigned i = 1; i < numTypes; ++i) {
        if (i > 1)
            oss << ",";
        oss << resultTypeAndParams[i]->description;
    }
    oss << "): " << resultTypeAndParams[0]->description;
    return oss.str();
}
} // namespace

DEFINE_TYPE_COMMON_IMPL(VoidType, Type)

VoidType VoidType::changeTypeModeImpl(VoidType /*type*/, Nest::EvalMode newMode) {
    return VoidType::get(newMode);
}

VoidType VoidType::get(Nest::EvalMode mode) {
    Nest_Type referenceType = {0};
    referenceType.typeKind = typeKindVoid;
    referenceType.mode = mode;
    referenceType.numSubtypes = 0;
    referenceType.numReferences = 0;
    referenceType.hasStorage = 0;
    referenceType.canBeUsedAtRt = 1;
    referenceType.flags = 0;
    referenceType.referredNode = nullptr;

    TypeRef t = Nest_findStockType(&referenceType);
    if (!t) {
        referenceType.description = getVoidDescription(mode);
        t = Nest_insertStockType(&referenceType);
    }
    return VoidType(t);
}

VoidType VoidType::changeMode(Nest::EvalMode mode, Nest::Location /*loc*/) const {
    return get(mode); // No checks here. Always succeeds
}

DEFINE_TYPE_COMMON_IMPL(DataType, TypeWithStorage)

DataType DataType::changeTypeModeImpl(DataType type, Nest::EvalMode newMode) {
    return DataType::get(type.referredNode(), newMode);
}

DataType DataType::get(Nest::NodeHandle decl, Nest::EvalMode mode) {
    ASSERT(decl.kind() == nkFeatherDeclClass);
    EvalMode classMode = decl ? Feather_effectiveEvalMode(decl) : mode;
    if (mode == modeRt && decl)
        mode = classMode;

    Nest_Type referenceType = {0};
    referenceType.typeKind = typeKindData;
    referenceType.mode = mode;
    referenceType.numSubtypes = 0;
    referenceType.numReferences = 0;
    referenceType.hasStorage = 1;
    referenceType.canBeUsedAtRt = classMode != modeCt;
    referenceType.flags = 0;
    referenceType.referredNode = decl;

    TypeRef t = Nest_findStockType(&referenceType);
    if (!t) {
        referenceType.description = str(getDataTypeDescription(decl, mode));
        t = Nest_insertStockType(&referenceType);
    }
    return {t};
}

DEFINE_TYPE_COMMON_IMPL(PtrType, TypeWithStorage)

PtrType PtrType::changeTypeModeImpl(PtrType type, Nest::EvalMode newMode) {
    return PtrType::get(type.base().changeMode(newMode));
}

PtrType PtrType::get(TypeWithStorage base, Nest::Location loc) {
    if (!base)
        REP_INTERNAL(loc, "Null type given as base to ptr type");

    Nest_Type referenceType = {0};
    referenceType.typeKind = typeKindPtr;
    referenceType.mode = base.mode();
    referenceType.numSubtypes = 1;
    referenceType.numReferences = 1 + base.numReferences();
    referenceType.hasStorage = 1;
    referenceType.canBeUsedAtRt = base.canBeUsedAtRt();
    referenceType.flags = 0;
    referenceType.referredNode = base.referredNode();

    // Temporarily use the pointer to the given parameter
    referenceType.subTypes = &base.type_;

    TypeRef t = Nest_findStockType(&referenceType);
    if (!t) {
        referenceType.description = str(getPtrTypeDescription(base));
        // Allocate now new buffer to hold the subtypes
        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
        referenceType.subTypes = new TypeRef[1];
        referenceType.subTypes[0] = base;

        t = Nest_insertStockType(&referenceType);
    }
    return {t};
}

DEFINE_TYPE_COMMON_IMPL(ConstType, TypeWithStorage)

ConstType ConstType::changeTypeModeImpl(ConstType type, Nest::EvalMode newMode) {
    return ConstType::get(type.base().changeMode(newMode));
}

ConstType ConstType::get(TypeWithStorage base, Nest::Location loc) {
    if (!base)
        REP_INTERNAL(loc, "Null type given as base to const type");
    int baseKind = base.kind();
    if (baseKind == typeKindConst)
        return {base};
    else if (baseKind == typeKindMutable || baseKind == typeKindTemp)
        REP_ERROR(loc, "Cannot construct a const type based on %1%") % base;
    Nest_Type referenceType = {0};
    referenceType.typeKind = typeKindConst;
    referenceType.mode = base.mode();
    referenceType.numSubtypes = 1;
    referenceType.numReferences = 1 + base.numReferences();
    referenceType.hasStorage = 1;
    referenceType.canBeUsedAtRt = base.canBeUsedAtRt();
    referenceType.flags = 0;
    referenceType.referredNode = base.referredNode();
    referenceType.description = str(getConstTypeDescription(base));

    // Temporarily use the pointer to the given parameter
    referenceType.subTypes = &base.type_;

    TypeRef t = Nest_findStockType(&referenceType);
    if (!t) {
        // Allocate now new buffer to hold the subtypes
        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
        referenceType.subTypes = new TypeRef[1];
        referenceType.subTypes[0] = base;

        t = Nest_insertStockType(&referenceType);
    }
    return {t};
}

TypeWithStorage ConstType::toRef() const {
    return PtrType::get(base());
}

DEFINE_TYPE_COMMON_IMPL(MutableType, TypeWithStorage)

MutableType MutableType::changeTypeModeImpl(MutableType type, Nest::EvalMode newMode) {
    return MutableType::get(type.base().changeMode(newMode));
}

MutableType MutableType::get(TypeWithStorage base, Nest::Location loc) {
    if (!base)
        REP_INTERNAL(loc, "Null type given as base to mutable type");
    int baseKind = base.kind();
    if (baseKind == typeKindMutable)
        return {base};
    else if (baseKind == typeKindConst || baseKind == typeKindTemp)
        REP_ERROR(loc, "Cannot construct a mutable type based on %1%") % base;

    Nest_Type referenceType = {0};
    referenceType.typeKind = typeKindMutable;
    referenceType.mode = base.mode();
    referenceType.numSubtypes = 1;
    referenceType.numReferences = 1 + base.numReferences();
    referenceType.hasStorage = 1;
    referenceType.canBeUsedAtRt = base.canBeUsedAtRt();
    referenceType.flags = 0;
    referenceType.referredNode = base.referredNode();
    referenceType.description = str(getMutableTypeDescription(base));

    // Temporarily use the pointer to the given parameter
    referenceType.subTypes = &base.type_;

    TypeRef t = Nest_findStockType(&referenceType);
    if (!t) {
        // Allocate now new buffer to hold the subtypes
        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
        referenceType.subTypes = new TypeRef[1];
        referenceType.subTypes[0] = base;

        t = Nest_insertStockType(&referenceType);
    }
    return {t};
}

TypeWithStorage MutableType::toRef() const {
    return PtrType::get(base());
}

DEFINE_TYPE_COMMON_IMPL(TempType, TypeWithStorage)

TempType TempType::changeTypeModeImpl(TempType type, Nest::EvalMode newMode) {
    return TempType::get(type.base().changeMode(newMode));
}

TempType TempType::get(TypeWithStorage base, Nest::Location loc) {
    if (!base)
        REP_INTERNAL(loc, "Null type given as base to const type");
    int baseKind = base.kind();
    if (baseKind == typeKindTemp)
        return {base};
    else if (baseKind == typeKindConst || baseKind == typeKindMutable)
        REP_ERROR(loc, "Cannot construct a tmp type based on %1%") % base;

    Nest_Type referenceType = {0};
    referenceType.typeKind = typeKindTemp;
    referenceType.mode = base.mode();
    referenceType.numSubtypes = 1;
    referenceType.numReferences = 1 + base.numReferences();
    referenceType.hasStorage = 1;
    referenceType.canBeUsedAtRt = base.canBeUsedAtRt();
    referenceType.flags = 0;
    referenceType.referredNode = base.referredNode();
    referenceType.description = str(getTempTypeDescription(base));

    // Temporarily use the pointer to the given parameter
    referenceType.subTypes = &base.type_;

    TypeRef t = Nest_findStockType(&referenceType);
    if (!t) {
        // Allocate now new buffer to hold the subtypes
        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
        referenceType.subTypes = new TypeRef[1];
        referenceType.subTypes[0] = base;

        t = Nest_insertStockType(&referenceType);
    }
    return {t};
}

TypeWithStorage TempType::toRef() const {
    return PtrType::get(base());
}

DEFINE_TYPE_COMMON_IMPL(ArrayType, TypeWithStorage)

ArrayType ArrayType::changeTypeModeImpl(ArrayType type, Nest::EvalMode newMode) {
    return ArrayType::get(type.unitType().changeMode(newMode), type.count());
}

ArrayType ArrayType::get(TypeWithStorage unitType, int count) {
    Nest_Type referenceType = {0};
    referenceType.typeKind = typeKindArray;
    referenceType.mode = unitType.mode();
    referenceType.numSubtypes = 1;
    referenceType.numReferences = 0;
    referenceType.hasStorage = 1;
    referenceType.canBeUsedAtRt = unitType.canBeUsedAtRt();
    referenceType.flags = count;
    referenceType.referredNode = unitType.referredNode();
    referenceType.description = str(getArrayTypeDescription(unitType, count));

    // Temporarily use the pointer to the given parameter
    referenceType.subTypes = &unitType.type_;

    TypeRef t = Nest_findStockType(&referenceType);
    if (!t) {
        // Allocate now new buffer to hold the subtypes
        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
        referenceType.subTypes = new TypeRef[1];
        referenceType.subTypes[0] = unitType;

        t = Nest_insertStockType(&referenceType);
    }
    return {t};
}

DEFINE_TYPE_COMMON_IMPL(FunctionType, TypeWithStorage)

FunctionType FunctionType::changeTypeModeImpl(FunctionType type, Nest::EvalMode newMode) {
    return FunctionType::get(type.type_->subTypes, type.type_->numSubtypes, newMode);
}

FunctionType FunctionType::get(
        Nest::TypeRef* resultTypeAndParams, int numTypes, Nest::EvalMode mode) {
    ASSERT(numTypes >= 1); // At least result type

    Nest_Type referenceType = {0};
    referenceType.typeKind = typeKindFunction;
    referenceType.mode = mode;
    referenceType.numSubtypes = numTypes;
    referenceType.numReferences = 0;
    referenceType.hasStorage = 1;
    referenceType.canBeUsedAtRt = 1;
    referenceType.flags = 0;
    referenceType.referredNode = nullptr;
    referenceType.description =
            str(Feather_getFunctionTypeDescription(resultTypeAndParams, numTypes, mode));

    // Temporarily use the given array pointer
    referenceType.subTypes = resultTypeAndParams;

    TypeRef t = Nest_findStockType(&referenceType);
    if (!t) {
        // Allocate now new buffer to hold the subtypes
        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
        referenceType.subTypes = new TypeRef[numTypes];
        copy(resultTypeAndParams, resultTypeAndParams + numTypes, referenceType.subTypes);

        t = Nest_insertStockType(&referenceType);
    }
    return {t};
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

TypeWithStorage addRef(TypeWithStorage type) {
    if (!type)
        REP_INTERNAL(NOLOC, "Null type passed to addRef");

    int typeKind = type.kind();
    if (typeKind == typeKindData)
        return PtrType::get(type);
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
        REP_INTERNAL(NOLOC, "Cannot remove reference from %1%") % type;
        return {};
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
    if (typeKind == typeKindConst)
        return ConstType(type).base();
    else if (typeKind == typeKindMutable)
        return MutableType(type).base();
    else if (typeKind == typeKindTemp)
        return TempType(type).base();
    else if (typeKind == typeKindPtr)
        return PtrType(type).base();

    REP_INTERNAL(NOLOC, "Invalid type given when removing reference (%1%)") % type;
    return {};
}

TypeWithStorage removeAllRefs(TypeWithStorage type) {
    if (!type)
        REP_INTERNAL(NOLOC, "Null type passed to removeAllRefs");

    int typeKind = type.kind();
    if (typeKind == typeKindData || typeKind == typeKindPtr || typeKind == typeKindConst ||
            typeKind == typeKindMutable || typeKind == typeKindTemp)
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
    if (baseKind != typeKindData && baseKind != typeKindPtr && baseKind != typeKindConst &&
            baseKind != typeKindMutable && baseKind != typeKindTemp)
        REP_INTERNAL(NOLOC, "Invalid type given to change category: %1%") % type;

    TypeWithStorage base = removeCategoryIfPresent(type);

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
