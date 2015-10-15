#include "Nest/src/StdInc.hpp"
#include "Nest/Api/NodeKindRegistrar.h"
#include "Nest/Api/Node.h"

namespace
{
    struct NodeKindDescription
    {
        const char* name;
        FSemanticCheck funSemanticCheck;
        FComputeType funComputeType;
        FSetContextForChildren funSetContextForChildren;
        FToString funToString;
    };

    /// The registered node kinds
    static vector<NodeKindDescription> registeredNodeKinds;
}

int Nest_registerNodeKind(const char* name,
                           FSemanticCheck funSemanticCheck,
                           FComputeType funComputeType,
                           FSetContextForChildren funSetContextForChildren,
                           FToString funToString)
{
    int nodeKindId = registeredNodeKinds.size();
    if ( !funComputeType )              funComputeType = Nest_defaultFunComputeType;
    if ( !funSetContextForChildren )    funSetContextForChildren = Nest_defaultFunSetContextForChildren;
    if ( !funToString )                 funToString = Nest_defaultFunToString;
    NodeKindDescription nk = { name, funSemanticCheck, funComputeType, funSetContextForChildren, funToString };
    registeredNodeKinds.push_back(nk);
    return nodeKindId;
}


const char* Nest_getNodeKindName(int nodeKind)
{
//    ASSERT(0 <= nodeKind  && nodeKind < registeredNodeKinds.size());
    return registeredNodeKinds[nodeKind].name;
}

FSemanticCheck Nest_getSemanticCheckFun(int nodeKind)
{
    ASSERT(0 <= nodeKind  && nodeKind < registeredNodeKinds.size());
    return registeredNodeKinds[nodeKind].funSemanticCheck;
}

FComputeType Nest_getComputeTypeFun(int nodeKind)
{
    ASSERT(0 <= nodeKind  && nodeKind < registeredNodeKinds.size());
    return registeredNodeKinds[nodeKind].funComputeType;
}

FSetContextForChildren Nest_getSetContextForChildrenFun(int nodeKind)
{
    ASSERT(0 <= nodeKind  && nodeKind < registeredNodeKinds.size());
    return registeredNodeKinds[nodeKind].funSetContextForChildren;
}

FToString Nest_getToStringFun(int nodeKind)
{
    ASSERT(0 <= nodeKind  && nodeKind < registeredNodeKinds.size());
    return registeredNodeKinds[nodeKind].funToString;
}

