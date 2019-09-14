#include "Feather/src/StdInc.h"
#include "Feather/src/Api/Feather_Types.h"
#include "Feather/Api/Feather.h"
#include "Feather/Utils/FeatherUtils.hpp"

#include "Nest/Api/TypeKindRegistrar.h"
#include "Nest/Api/Node.h"
#include "Nest/Utils/NodeUtils.h"
#include "Nest/Utils/Alloc.h"

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
string getDataTypeDescription(Node* classDecl, unsigned numReferences, EvalMode mode) {
    string res;
    for (unsigned i = 0; i < numReferences; ++i)
        res += '@';
    if (classDecl) {
        const Nest_StringRef* description = Nest_getPropertyString(classDecl, propDescription);
        StringRef desc = description ? *description : Feather_getName(classDecl);
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

TypeRef changeTypeModeVoid(TypeRef type, EvalMode newMode) { return Feather_getVoidType(newMode); }
TypeRef changeTypeModeData(TypeRef type, EvalMode newMode) {
    return Feather_getDataType(type->referredNode, type->numReferences, newMode);
}
TypeRef changeTypeModePtr(TypeRef type, EvalMode newMode) {
    return Feather_getPtrType(Nest_changeTypeMode(Feather_baseType(type), newMode));
}
TypeRef changeTypeModeConst(TypeRef type, EvalMode newMode) {
    return Feather_getConstType(Nest_changeTypeMode(Feather_baseType(type), newMode));
}
TypeRef changeTypeModeMutable(TypeRef type, EvalMode newMode) {
    return Feather_getMutableType(Nest_changeTypeMode(Feather_baseType(type), newMode));
}
TypeRef changeTypeModeTemp(TypeRef type, EvalMode newMode) {
    return Feather_getTempType(Nest_changeTypeMode(Feather_baseType(type), newMode));
}
TypeRef changeTypeModeArray(TypeRef type, EvalMode newMode) {
    return Feather_getArrayType(
            Nest_changeTypeMode(Feather_baseType(type), newMode), Feather_getArraySize(type));
}
TypeRef changeTypeModeFunction(TypeRef type, EvalMode newMode) {
    return Feather_getFunctionType(type->subTypes, type->numSubtypes, newMode);
}
} // namespace

int typeKindVoid = -1;
int typeKindData = -1;
int typeKindPtr = -1;
int typeKindConst = -1;
int typeKindMutable = -1;
int typeKindTemp = -1;
int typeKindArray = -1;
int typeKindFunction = -1;

void initFeatherTypeKinds() {
    typeKindVoid = Nest_registerTypeKind(&changeTypeModeVoid);
    typeKindData = Nest_registerTypeKind(&changeTypeModeData);
    typeKindPtr = Nest_registerTypeKind(&changeTypeModePtr);
    typeKindConst = Nest_registerTypeKind(&changeTypeModeConst);
    typeKindMutable = Nest_registerTypeKind(&changeTypeModeMutable);
    typeKindTemp = Nest_registerTypeKind(&changeTypeModeTemp);
    typeKindArray = Nest_registerTypeKind(&changeTypeModeArray);
    typeKindFunction = Nest_registerTypeKind(&changeTypeModeFunction);
}

int Feather_getVoidTypeKind() { return typeKindVoid; }
int Feather_getDataTypeKind() { return typeKindData; }
int Feather_getPtrTypeKind() { return typeKindPtr; }
int Feather_getConstTypeKind() { return typeKindConst; }
int Feather_getMutableTypeKind() { return typeKindMutable; }
int Feather_getTempTypeKind() { return typeKindTemp; }
int Feather_getArrayTypeKind() { return typeKindArray; }
int Feather_getFunctionTypeKind() { return typeKindFunction; }

TypeRef Feather_getVoidType(EvalMode mode) {
    Nest_Type referenceType = {0};
    referenceType.typeKind = typeKindVoid;
    referenceType.mode = mode;
    referenceType.numSubtypes = 0;
    referenceType.numReferences = 0;
    referenceType.hasStorage = 0;
    referenceType.canBeUsedAtRt = 1;
    referenceType.flags = 0;
    referenceType.referredNode = nullptr;
    referenceType.description = getVoidDescription(mode);

    TypeRef t = Nest_findStockType(&referenceType);
    if (!t)
        t = Nest_insertStockType(&referenceType);
    return t;
}

TypeRef Feather_getDataType(Node* classDecl, unsigned numReferences, EvalMode mode) {
    ASSERT(classDecl->nodeKind == nkFeatherDeclClass);
    EvalMode classMode = classDecl ? Feather_effectiveEvalMode(classDecl) : mode;
    if (mode == modeRt && classDecl)
        mode = classMode;

    Nest_Type referenceType = {0};
    referenceType.typeKind = typeKindData;
    referenceType.mode = mode;
    referenceType.numSubtypes = 0;
    referenceType.numReferences = numReferences;
    referenceType.hasStorage = 1;
    referenceType.canBeUsedAtRt = classMode != modeCt;
    referenceType.flags = 0;
    referenceType.referredNode = classDecl;
    referenceType.description = str(getDataTypeDescription(classDecl, numReferences, mode));

    TypeRef t = Nest_findStockType(&referenceType);
    if (!t)
        t = Nest_insertStockType(&referenceType);
    return t;
}

TypeRef Feather_getPtrType(TypeRef base) {
    Nest_Type referenceType = {0};
    referenceType.typeKind = typeKindPtr;
    referenceType.mode = base->mode;
    referenceType.numSubtypes = 1;
    referenceType.numReferences = 1 + base->numReferences;
    referenceType.hasStorage = 1;
    referenceType.canBeUsedAtRt = base->canBeUsedAtRt;
    referenceType.flags = 0;
    referenceType.referredNode = base->referredNode;
    referenceType.description = str(getPtrTypeDescription(base));

    // Temporarily use the pointer to the given parameter
    referenceType.subTypes = &base;

    TypeRef t = Nest_findStockType(&referenceType);
    if (!t) {
        // Allocate now new buffer to hold the subtypes
        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
        referenceType.subTypes = new TypeRef[1];
        referenceType.subTypes[0] = base;

        t = Nest_insertStockType(&referenceType);
    }
    return t;
}

TypeRef Feather_getConstType(TypeRef base) {
    Nest_Type referenceType = {0};
    referenceType.typeKind = typeKindConst;
    referenceType.mode = base->mode;
    referenceType.numSubtypes = 1;
    referenceType.numReferences = 1 + base->numReferences;
    referenceType.hasStorage = 1;
    referenceType.canBeUsedAtRt = base->canBeUsedAtRt;
    referenceType.flags = 0;
    referenceType.referredNode = base->referredNode;
    referenceType.description = str(getConstTypeDescription(base));

    // Temporarily use the pointer to the given parameter
    referenceType.subTypes = &base;

    TypeRef t = Nest_findStockType(&referenceType);
    if (!t) {
        // Allocate now new buffer to hold the subtypes
        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
        referenceType.subTypes = new TypeRef[1];
        referenceType.subTypes[0] = base;

        t = Nest_insertStockType(&referenceType);
    }
    return t;
}

TypeRef Feather_getMutableType(TypeRef base) {
    Nest_Type referenceType = {0};
    referenceType.typeKind = typeKindMutable;
    referenceType.mode = base->mode;
    referenceType.numSubtypes = 1;
    referenceType.numReferences = 1 + base->numReferences;
    referenceType.hasStorage = 1;
    referenceType.canBeUsedAtRt = base->canBeUsedAtRt;
    referenceType.flags = 0;
    referenceType.referredNode = base->referredNode;
    referenceType.description = str(getMutableTypeDescription(base));

    // Temporarily use the pointer to the given parameter
    referenceType.subTypes = &base;

    TypeRef t = Nest_findStockType(&referenceType);
    if (!t) {
        // Allocate now new buffer to hold the subtypes
        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
        referenceType.subTypes = new TypeRef[1];
        referenceType.subTypes[0] = base;

        t = Nest_insertStockType(&referenceType);
    }
    return t;
}

TypeRef Feather_getTempType(TypeRef base) {
    Nest_Type referenceType = {0};
    referenceType.typeKind = typeKindTemp;
    referenceType.mode = base->mode;
    referenceType.numSubtypes = 1;
    referenceType.numReferences = 1 + base->numReferences;
    referenceType.hasStorage = 1;
    referenceType.canBeUsedAtRt = base->canBeUsedAtRt;
    referenceType.flags = 0;
    referenceType.referredNode = base->referredNode;
    referenceType.description = str(getTempTypeDescription(base));

    // Temporarily use the pointer to the given parameter
    referenceType.subTypes = &base;

    TypeRef t = Nest_findStockType(&referenceType);
    if (!t) {
        // Allocate now new buffer to hold the subtypes
        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
        referenceType.subTypes = new TypeRef[1];
        referenceType.subTypes[0] = base;

        t = Nest_insertStockType(&referenceType);
    }
    return t;
}

TypeRef Feather_getArrayType(TypeRef unitType, unsigned count) {
    Nest_Type referenceType = {0};
    referenceType.typeKind = typeKindArray;
    referenceType.mode = unitType->mode;
    referenceType.numSubtypes = 1;
    referenceType.numReferences = 0;
    referenceType.hasStorage = 1;
    referenceType.canBeUsedAtRt = unitType->canBeUsedAtRt;
    referenceType.flags = count;
    referenceType.referredNode = unitType->referredNode;
    referenceType.description = str(getArrayTypeDescription(unitType, count));

    // Temporarily use the pointer to the given parameter
    referenceType.subTypes = &unitType;

    TypeRef t = Nest_findStockType(&referenceType);
    if (!t) {
        // Allocate now new buffer to hold the subtypes
        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
        referenceType.subTypes = new TypeRef[1];
        referenceType.subTypes[0] = unitType;

        t = Nest_insertStockType(&referenceType);
    }
    return t;
}

TypeRef Feather_getFunctionType(TypeRef* resultTypeAndParams, unsigned numTypes, EvalMode mode) {
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
    return t;
}
