#pragma once

#include "TypeRef.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Nest_Node Nest_Node;

/// Function that gets a string description for the given node
typedef const char* (*FToString)(Nest_Node* node);
/// Function that sets the context for the children of this node
typedef void (*FSetContextForChildren)(Nest_Node* node);
/// Function that computes the type of the given node
typedef Nest_TypeRef (*FComputeType)(Nest_Node* node);
/// Function that semantically checks the given node
typedef Nest_Node* (*FSemanticCheck)(Nest_Node* node);

/// Registers a new node kind
///
/// @return the ID of the new node kind
int Nest_registerNodeKind(const char* name, FSemanticCheck funSemanticCheck,
        FComputeType funComputeType, FSetContextForChildren funSetContextForChildren,
        FToString funToString);

const char* Nest_getNodeKindName(int nodeKind);
FSemanticCheck Nest_getSemanticCheckFun(int nodeKind);
FComputeType Nest_getComputeTypeFun(int nodeKind);
FSetContextForChildren Nest_getSetContextForChildrenFun(int nodeKind);
FToString Nest_getToStringFun(int nodeKind);

//! Resets the registered node kinds
void Nest_resetRegisteredNodeKinds();

#ifdef __cplusplus
}
#endif
