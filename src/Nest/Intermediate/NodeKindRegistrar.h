#pragma once

#include "TypeRef.h"

FWD_STRUCT1(Nest, Node);

namespace Nest
{
    /// Function that gets a string description for the given node
    typedef const char* (*FToString)(Node* node);
    /// Function that sets the context for the children of this node
    typedef void (*FSetContextForChildren)(Node* node);
    /// Function that computes the type of the given node
    typedef TypeRef (*FComputeType)(Node* node);
    /// Function that semantically checks the given node
    typedef Node* (*FSemanticCheck)(Node* node);

    /// Registers a new node kind
    ///
    /// @return the ID of the new node kind
    int registerNodeKind(const char* name,
                         FSemanticCheck funSemanticCheck,
                         FComputeType funComputeType = NULL,
                         FSetContextForChildren funSetContextForChildren = NULL,
                         FToString funToString = NULL);

    const char* getNodeKindName(int nodeKind);
    FSemanticCheck getSemanticCheckFun(int nodeKind);
    FComputeType getComputeTypeFun(int nodeKind);
    FSetContextForChildren getSetContextForChildrenFun(int nodeKind);
    FToString getToStringFun(int nodeKind);
}
