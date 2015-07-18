#include <StdInc.h>
#include "NodeKindRegistrar.h"
#include "Node.h"

using namespace Nest;

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

int Nest::registerNodeKind(const char* name,
                           FSemanticCheck funSemanticCheck,
                           FComputeType funComputeType,
                           FSetContextForChildren funSetContextForChildren,
                           FToString funToString)
{
    int nodeKindId = registeredNodeKinds.size();
    if ( !funComputeType )              funComputeType = defaultFunComputeType;
    if ( !funSetContextForChildren )    funSetContextForChildren = defaultFunSetContextForChildren;
    if ( !funToString )                 funToString = defaultFunToString;
    NodeKindDescription nk = { name, funSemanticCheck, funComputeType, funSetContextForChildren, funToString };
    registeredNodeKinds.push_back(nk);
    return nodeKindId;
}


const char* Nest::getNodeKindName(int nodeKind)
{
//    ASSERT(0 <= nodeKind  && nodeKind < registeredNodeKinds.size());
    return registeredNodeKinds[nodeKind].name;
}

FSemanticCheck Nest::getSemanticCheckFun(int nodeKind)
{
    ASSERT(0 <= nodeKind  && nodeKind < registeredNodeKinds.size());
    return registeredNodeKinds[nodeKind].funSemanticCheck;
}

FComputeType Nest::getComputeTypeFun(int nodeKind)
{
    ASSERT(0 <= nodeKind  && nodeKind < registeredNodeKinds.size());
    return registeredNodeKinds[nodeKind].funComputeType;
}

FSetContextForChildren Nest::getSetContextForChildrenFun(int nodeKind)
{
    ASSERT(0 <= nodeKind  && nodeKind < registeredNodeKinds.size());
    return registeredNodeKinds[nodeKind].funSetContextForChildren;
}

FToString Nest::getToStringFun(int nodeKind)
{
    ASSERT(0 <= nodeKind  && nodeKind < registeredNodeKinds.size());
    return registeredNodeKinds[nodeKind].funToString;
}

