#pragma once

#include "TypeRef.h"

typedef struct Nest_Node Node;

/// Function that gets a string description for the given node
typedef const char* (*FToString)(const Node* node);
/// Function that sets the context for the children of this node
typedef void (*FSetContextForChildren)(Node* node);
/// Function that computes the type of the given node
typedef TypeRef (*FComputeType)(Node* node);
/// Function that semantically checks the given node
typedef Node* (*FSemanticCheck)(Node* node);

/// Registers a new node kind
///
/// @return the ID of the new node kind
int Nest_registerNodeKind(const char* name,
                     FSemanticCheck funSemanticCheck,
                     FComputeType funComputeType = NULL,
                     FSetContextForChildren funSetContextForChildren = NULL,
                     FToString funToString = NULL);

const char* Nest_getNodeKindName(int nodeKind);
FSemanticCheck Nest_getSemanticCheckFun(int nodeKind);
FComputeType Nest_getComputeTypeFun(int nodeKind);
FSetContextForChildren Nest_getSetContextForChildrenFun(int nodeKind);
FToString Nest_getToStringFun(int nodeKind);
