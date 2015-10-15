#pragma once

#include "Nest/Api/TypeRef.h"
#include "Nest/Utils/StringRef.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Nest_Node Node;


/// Enumeration describing possible property kinds
enum Nest_PropertyKind {
    propInt,
    propString,
    propNode,
    propType,
};
typedef enum Nest_PropertyKind PropertyKind;

/// A node property, consisting of a name, a kind and a value
/// We also store here a flag indicating whether the property should be copied
/// to the explanation node
struct Nest_NodeProperty {
    StringRef name;
    PropertyKind kind: 16;
    int passToExpl: 1;
    union {
        int intValue;
        StringRef stringValue;
        Node* nodeValue;
        TypeRef typeValue;
    } value;
};
typedef struct Nest_NodeProperty Nest_NodeProperty;
typedef struct Nest_NodeProperty NodeProperty;

/// Structure holding the properties of a node
/// We store them in an array-like structure
struct Nest_NodeProperties {
    NodeProperty* begin;
    NodeProperty* end;
    NodeProperty* endOfStore;
};

typedef struct Nest_NodeProperties Nest_NodeProperties;
typedef struct Nest_NodeProperties NodeProperties;

void Nest_addProperty(NodeProperties* properties, NodeProperty prop);

#ifdef __cplusplus
}
#endif
