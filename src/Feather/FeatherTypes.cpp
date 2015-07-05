#include <StdInc.h>
#include "FeatherTypes.h"
#include "Nodes/Properties.h"
#include "Nodes/Decls/Class.h"
#include "Util/Decl.h"

namespace Feather
{

namespace
{
    const char* str(const string& s)
    {
        return strdup(s.c_str());
    }

    const char* getVoidDescription(EvalMode mode)
    {
        switch ( mode )
        {
        case modeCt:    return "Void/ct";
        case modeRtCt:  return "Void/rtct";
        default:        return "Void";
        }
    }
    string getDataTypeDescription(Node* classDecl, uint8_t numReferences, EvalMode mode)
    {
        string res;
        for ( uint8_t i=0; i<numReferences; ++i )
            res += '@';
        if ( classDecl )
        {
            const string* description = classDecl->getPropertyString(propDescription);
            res += description ? *description : getName(classDecl);
        }
        else
            res += "<no class>";
        if ( mode == modeCt )
            res += "/ct";
        if ( mode == modeRtCt )
            res += "/rtct";
        return res;
    }
    string getLValueTypeDescription(TypeRef base)
    {
        return string(base->description) + " lv";
    }
    string getArrayTypeDescription(TypeRef unitType, uint32_t count)
    {
        ostringstream oss;
        oss << unitType->description << " A(" << count << ")";
        return oss.str();
    }
    string getFunctionTypeDescription(TypeRef* resultTypeAndParams, size_t numTypes, EvalMode mode)
    {
        ostringstream oss;
        oss << "F(";
        for ( size_t i=1; i<numTypes; ++i )
        {
            if ( i > 1 )
                oss << ",";
            oss << resultTypeAndParams[i];
        }
        oss << "): " << resultTypeAndParams[0];
        return oss.str();
    }
}

TypeRef getVoidType(EvalMode mode)
{
    Type referenceType;
    referenceType.typeKind        = Nest::typeVoid;
    referenceType.mode          = mode;
    referenceType.numSubtypes   = 0;
    referenceType.numReferences = 0;
    referenceType.hasStorage    = 0;
    referenceType.canBeUsedAtCt = 1;
    referenceType.canBeUsedAtRt = 1;
    referenceType.flags         = 0;
    referenceType.referredNode  = nullptr;
    referenceType.description   = getVoidDescription(mode);

    TypeRef t = findStockType(referenceType);
    if ( !t )
        t = insertStockType(referenceType);
    return t;
}

TypeRef getDataType(Class* classDecl, uint8_t numReferences, EvalMode mode)
{
    EvalMode classMode = classDecl ? effectiveEvalMode(classDecl) : mode;
    if ( mode == modeRtCt && classDecl )
        mode = classMode;

    Type referenceType;
    referenceType.typeKind        = Nest::typeData;
    referenceType.mode          = mode;
    referenceType.numSubtypes   = 0;
    referenceType.numReferences = numReferences;
    referenceType.hasStorage    = 1;
    referenceType.canBeUsedAtCt = classMode != modeRt;
    referenceType.canBeUsedAtRt = classMode != modeCt;
    referenceType.flags         = 0;
    referenceType.referredNode  = classDecl;
    referenceType.description   = str(getDataTypeDescription(classDecl, numReferences, mode));

    TypeRef t = findStockType(referenceType);
    if ( !t )
        t = insertStockType(referenceType);
    return t;
}

TypeRef getLValueType(TypeRef base)
{
    Type referenceType;
    referenceType.typeKind        = Nest::typeLValue;
    referenceType.mode          = base->mode;
    referenceType.numSubtypes   = 1;
    referenceType.numReferences = 1+base->numReferences;
    referenceType.hasStorage    = 1;
    referenceType.canBeUsedAtCt = base->canBeUsedAtCt;
    referenceType.canBeUsedAtRt = base->canBeUsedAtRt;
    referenceType.flags         = 0;
    referenceType.referredNode  = base->referredNode;
    referenceType.description   = str(getLValueTypeDescription(base));

    // Temporarily use the pointer to the given parameter
    referenceType.subTypes = &base;

    TypeRef t = findStockType(referenceType);
    if ( !t )
    {
        // Allocate now new buffer to hold the subtypes
        referenceType.subTypes = new TypeRef[1];
        referenceType.subTypes[0] = base;

        t = insertStockType(referenceType);
    }
    return t;
}

TypeRef getArrayType(TypeRef unitType, uint32_t count)
{
    Type referenceType;
    referenceType.typeKind        = Nest::typeArray;
    referenceType.mode          = unitType->mode;
    referenceType.numSubtypes   = 1;
    referenceType.numReferences = 0;
    referenceType.hasStorage    = 1;
    referenceType.canBeUsedAtCt = unitType->canBeUsedAtCt;
    referenceType.canBeUsedAtRt = unitType->canBeUsedAtRt;
    referenceType.flags         = count;
    referenceType.referredNode  = unitType->referredNode;
    referenceType.description   = str(getArrayTypeDescription(unitType, count));

    // Temporarily use the pointer to the given parameter
    referenceType.subTypes = &unitType;

    TypeRef t = findStockType(referenceType);
    if ( !t )
    {
        // Allocate now new buffer to hold the subtypes
        referenceType.subTypes = new TypeRef[1];
        referenceType.subTypes[0] = unitType;

        t = insertStockType(referenceType);
    }
    return t;
}

TypeRef getFunctionType(TypeRef* resultTypeAndParams, size_t numTypes, EvalMode mode)
{
    ASSERT(numTypes >= 1);  // At least result type

    Type referenceType;
    referenceType.typeKind        = Nest::typeFunction;
    referenceType.mode          = mode;
    referenceType.numSubtypes   = numTypes;
    referenceType.numReferences = 0;
    referenceType.hasStorage    = 1;
    referenceType.canBeUsedAtCt = 1;
    referenceType.canBeUsedAtRt = 1;
    referenceType.flags         = 0;
    referenceType.referredNode  = nullptr;
    referenceType.description   = str(getFunctionTypeDescription(resultTypeAndParams, numTypes, mode));

    // Temporarily use the given array pointer
    referenceType.subTypes = resultTypeAndParams;

    TypeRef t = findStockType(referenceType);
    if ( !t )
    {
        // Allocate now new buffer to hold the subtypes
        referenceType.subTypes = new TypeRef[numTypes];
        copy(resultTypeAndParams, resultTypeAndParams+numTypes, referenceType.subTypes);

        t = insertStockType(referenceType);
    }
    return t;
}


Class* classDecl(TypeRef type)
{
    ASSERT(type && type->hasStorage);
    return type->referredNode->as<Class>();
}

const string* nativeName(TypeRef type)
{
    Class* cls = classDecl(type);
    return cls ? cls->getPropertyString(propNativeName) : nullptr;
}

int numReferences(TypeRef type)
{
    ASSERT(type && type->hasStorage);
    return type->numReferences;
}

TypeRef baseType(TypeRef type)
{
    ASSERT(type && (type->typeKind == typeLValue || type->typeKind == typeArray) && type->numSubtypes == 1);
    return type->subTypes[0];
}

int getArraySize(TypeRef type)
{
    ASSERT(type && type->typeKind == typeArray);
    return type->flags;
}

size_t numFunParameters(TypeRef type)
{
    ASSERT(type && type->typeKind == typeFunction);
    return type->numSubtypes-1;
}

TypeRef getFunParameter(TypeRef type, size_t idx)
{
    ASSERT(type && type->typeKind == typeFunction);
    return type->subTypes[idx+1];
}

vector<TypeRef> getFunParameters(TypeRef type)
{
    ASSERT(type && type->typeKind == typeFunction);
    return vector<TypeRef>(type->subTypes+1, type->subTypes+type->numSubtypes);
}

TypeRef getFunResultType(TypeRef type)
{
    ASSERT(type && type->typeKind == typeFunction);
    return type->subTypes[0];
}

}
