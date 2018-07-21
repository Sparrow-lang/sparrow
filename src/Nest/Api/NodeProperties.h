#pragma once

#include "Nest/Api/TypeRef.h"
#include "Nest/Api/StringRef.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Nest_Node Nest_Node;

/// Enumeration describing possible property kinds
enum Nest_PropertyKind {
    propInt,
    propString,
    propNode,
    propType,
    propPtr,
};
typedef enum Nest_PropertyKind Nest_PropertyKind;

/// A node property, consisting of a name, a kind and a value
/// We also store here a flag indicating whether the property should be copied
/// to the explanation node
struct Nest_NodeProperty {
    Nest_StringRef name;
    Nest_PropertyKind kind : 16;
    int passToExpl : 1;
    union {
        int intValue;
        Nest_StringRef stringValue;
        Nest_Node* nodeValue;
        Nest_TypeRef typeValue;
        void* ptrValue;
    } value;
};
typedef struct Nest_NodeProperty Nest_NodeProperty;

/// Structure holding the properties of a node
/// We store them in an array-like structure
struct Nest_NodeProperties {
    Nest_NodeProperty* begin;
    Nest_NodeProperty* end;
    Nest_NodeProperty* endOfStore;
};

typedef struct Nest_NodeProperties Nest_NodeProperties;

void Nest_addProperty(Nest_NodeProperties* properties, Nest_NodeProperty prop);

#ifdef __cplusplus
}
#endif
