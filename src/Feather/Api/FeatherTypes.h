#pragma once

#include "Nest/Api/TypeRef.h"
#include "Nest/Api/EvalMode.h"
#include "Nest/Api/StringRef.h"

typedef struct Nest_Node Node;

namespace Feather
{
    // The type kinds for the Feather types
    extern int typeKindVoid;
    extern int typeKindData;
    extern int typeKindLValue;
    extern int typeKindArray;
    extern int typeKindFunction;

    /// Called to initialize the Feather type kinds
    void initFeatherTypeKinds();

    /// Returns the Void type corresponding to the given evaluation mode
    TypeRef getVoidType(EvalMode mode);

    /// Returns a type that represents data
    /// A data type is introduced by a class definition and can have one or more references; a data type must have a size
    TypeRef getDataType(Node* classDecl, uint8_t numReferences = 0, EvalMode mode = modeRtCt);

    /// Returns an L-Value type
    /// This type would represents an l-value to an existing storage type
    TypeRef getLValueType(TypeRef base);

    /// Returns a type that holds N instances of a given storage type
    TypeRef getArrayType(TypeRef unitType, uint32_t count);

    /// Returns a type that represents a classic function, with parameters and result type
    /// This type can be constructed from a set of parameter types and a result type
    /// The first parameter is a pointer to an array with the result type then the types of the parameters
    TypeRef getFunctionType(TypeRef* resultTypeAndParams, size_t numTypes, EvalMode mode);


    /// Getter for the class that introduces this data type - can be null
    /// Works for only for storage types (data, l-value, array)
    Node* classDecl(TypeRef type);

    /// If the class associated with the given type has an associated name this will return it; otherwise it returns nullptr
    /// Works for only for storage types (data, l-value, array)
    const StringRef* nativeName(TypeRef type);

    /// The number of references applied
    /// Works for only for storage types (data, l-value, array)
    int numReferences(TypeRef type);

    /// Returns the base type of this type
    /// Works for l-value and array types
    TypeRef baseType(TypeRef type);

    /// Getter for the number of units in the buffer type
    /// Works only for array types
    int getArraySize(TypeRef type);

    size_t numFunParameters(TypeRef type);
    TypeRef getFunParameter(TypeRef type, size_t idx);
    vector<TypeRef> getFunParameters(TypeRef type);
    TypeRef getFunResultType(TypeRef type);
}
