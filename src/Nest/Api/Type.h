#pragma once

#include "TypeRef.h"
#include "EvalMode.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Nest_Node Nest_Node;

/// Represents a type
struct Nest_Type {
    unsigned typeKind : 8;       ///< The type ID
    EvalMode mode : 8;           ///< The evaluation mode of this type
    unsigned numSubtypes : 16;   ///< The number of subtypes of this type
    unsigned numReferences : 16; ///< The number of references of this type
    unsigned hasStorage : 1;     ///< True if this is type with storage
    unsigned canBeUsedAtRt : 1;  ///< Can we use this type at RT?
    unsigned flags : 32;         ///< Additional flags

    /// The subtypes of this type
    Nest_TypeRef* subTypes;

    /// Optional, the node that introduces this type
    Nest_Node* referredNode;

    /// The description of the type -- mainly used for debugging purposes
    const char* description;
};

typedef struct Nest_Type Nest_Type;

/// Get a stock type that matches the settings from the reference
/// We guarantee that the same types will have the same pointers
/// If we cannot find a type that matches the reference, this will return null
Nest_TypeRef Nest_findStockType(Nest_TypeRef reference);

/// Insert a new type into our stock
/// Returns the pointer to be used to represent this new type
Nest_TypeRef Nest_insertStockType(Nest_TypeRef newType);

/// Function that changes the mode for the given type
Nest_TypeRef Nest_changeTypeMode(Nest_TypeRef type, EvalMode newMode);

//! Reset all the cached types
void Nest_resetTypes();

#ifdef __cplusplus
}
#endif
