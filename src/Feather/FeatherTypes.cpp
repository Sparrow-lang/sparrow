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
    string getFunctionTypeDescription(TypeRef resultType, const vector<TypeRef>& paramTypes, EvalMode mode)
    {
        ostringstream oss;
        oss << "F(";
        for ( size_t i=0; i<paramTypes.size(); ++i )
        {
            if ( i > 0 )
                oss << ",";
            oss << paramTypes[i]->description;
        }
        oss << "): " << resultType->description;
        return oss.str();
    }
}

TypeRef getVoidType(EvalMode mode)
{
    TypeData referenceType;
    referenceType.typeId        = Nest::typeVoid;
    referenceType.mode          = mode;
    referenceType.numSubtypes   = 0;
    referenceType.numReferences = 0;
    referenceType.hasStorage    = 0;
    referenceType.canBeUsedAtCt = 1;
    referenceType.canBeUsedAtRt = 1;
    referenceType.flags         = 0;
    referenceType.referredNode  = nullptr;
    referenceType.description   = getVoidDescription(mode);
    return getStockType(referenceType);
}

TypeRef getDataType(Class* classDecl, uint8_t numReferences, EvalMode mode)
{
    EvalMode classMode = classDecl ? effectiveEvalMode(classDecl) : mode;
    if ( mode == modeRtCt && classDecl )
        mode = classMode;

    TypeData referenceType;
    referenceType.typeId        = Nest::typeData;
    referenceType.mode          = mode;
    referenceType.numSubtypes   = 0;
    referenceType.numReferences = numReferences;
    referenceType.hasStorage    = 1;
    referenceType.canBeUsedAtCt = classMode != modeRt;
    referenceType.canBeUsedAtRt = classMode != modeCt;
    referenceType.flags         = 0;
    referenceType.referredNode  = classDecl;
    referenceType.description   = str(getDataTypeDescription(classDecl, numReferences, mode));
    return getStockType(referenceType);
}

TypeRef getLValueType(TypeRef base)
{
    TypeData referenceType;
    referenceType.typeId        = Nest::typeLValue;
    referenceType.mode          = base->mode;
    referenceType.numSubtypes   = 1;
    referenceType.numReferences = 1+base->numReferences;
    referenceType.hasStorage    = 1;
    referenceType.canBeUsedAtCt = base->canBeUsedAtCt;
    referenceType.canBeUsedAtRt = base->canBeUsedAtRt;
    referenceType.flags         = 0;
    referenceType.referredNode  = base->referredNode;
    referenceType.description   = str(getLValueTypeDescription(base));

    referenceType.subTypes = new TypeRef[1];
    referenceType.subTypes[0] = base;

    return getStockType(referenceType);
}

TypeRef getArrayType(TypeRef unitType, uint32_t count)
{
    TypeData referenceType;
    referenceType.typeId        = Nest::typeArray;
    referenceType.mode          = unitType->mode;
    referenceType.numSubtypes   = 1;
    referenceType.numReferences = 0;
    referenceType.hasStorage    = 1;
    referenceType.canBeUsedAtCt = unitType->canBeUsedAtCt;
    referenceType.canBeUsedAtRt = unitType->canBeUsedAtRt;
    referenceType.flags         = count;
    referenceType.referredNode  = unitType->referredNode;
    referenceType.description   = str(getArrayTypeDescription(unitType, count));

    referenceType.subTypes = new TypeRef[1];
    referenceType.subTypes[0] = unitType;

    return getStockType(referenceType);
}

TypeRef getFunctionType(TypeRef resultType, const vector<TypeRef>& paramTypes, EvalMode mode)
{
    TypeData referenceType;
    referenceType.typeId        = Nest::typeFunction;
    referenceType.mode          = mode;
    referenceType.numSubtypes   = 1 + paramTypes.size();
    referenceType.numReferences = 0;
    referenceType.hasStorage    = 1;
    referenceType.canBeUsedAtCt = 1;
    referenceType.canBeUsedAtRt = 1;
    referenceType.flags         = 0;
    referenceType.referredNode  = nullptr;
    referenceType.description   = str(getFunctionTypeDescription(resultType, paramTypes, mode));

    referenceType.subTypes = new TypeRef[1+paramTypes.size()];
    referenceType.subTypes[0] = resultType;
    for ( size_t i=0; i<paramTypes.size(); ++i )
        referenceType.subTypes[1+i] = paramTypes[i];

    // TODO (types): Avoid allocating memory for types that are already in stock

    return getStockType(referenceType);
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
    ASSERT(type && (type->typeId == typeLValue || type->typeId == typeArray) && type->numSubtypes == 1);
    return type->subTypes[0];
}

int getArraySize(TypeRef type)
{
    ASSERT(type && type->typeId == typeArray);
    return type->flags;
}

size_t numFunParameters(TypeRef type)
{
    ASSERT(type && type->typeId == typeFunction);
    return type->numSubtypes-1;
}

TypeRef getFunParameter(TypeRef type, size_t idx)
{
    ASSERT(type && type->typeId == typeFunction);
    return type->subTypes[idx+1];
}

vector<TypeRef> getFunParameters(TypeRef type)
{
    ASSERT(type && type->typeId == typeFunction);
    return vector<TypeRef>(type->subTypes+1, type->subTypes+type->numSubtypes);
}

TypeRef getFunResultType(TypeRef type)
{
    ASSERT(type && type->typeId == typeFunction);
    return type->subTypes[0];
}

}
