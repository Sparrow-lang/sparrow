#include "Nest/Api/NodeKindRegistrar.h"
#include "Nest/Api/Node.h"
#include "Nest/Utils/Assert.h"

struct _NodeKindDescription {
    const char* name;
    FSemanticCheck funSemanticCheck;
    FComputeType funComputeType;
    FSetContextForChildren funSetContextForChildren;
    FToString funToString;
};

/// The registered node kinds
static const int _maxNodeKinds = 1000;
struct _NodeKindDescription _allNodeKinds[_maxNodeKinds];
unsigned int _numNodeKinds = 0;

int Nest_registerNodeKind(const char* name, FSemanticCheck funSemanticCheck,
        FComputeType funComputeType, FSetContextForChildren funSetContextForChildren,
        FToString funToString) {
    int nodeKindId = _numNodeKinds++;
    if (!funComputeType)
        funComputeType = Nest_defaultFunComputeType;
    if (!funSetContextForChildren)
        funSetContextForChildren = Nest_defaultFunSetContextForChildren;
    if (!funToString)
        funToString = Nest_defaultFunToString;
    struct _NodeKindDescription nk = {
            name, funSemanticCheck, funComputeType, funSetContextForChildren, funToString};
    _allNodeKinds[nodeKindId] = nk;
    return nodeKindId;
}

const char* Nest_getNodeKindName(int nodeKind) { return _allNodeKinds[nodeKind].name; }

FSemanticCheck Nest_getSemanticCheckFun(int nodeKind) {
    ASSERT(0 <= nodeKind && nodeKind < _numNodeKinds);
    return _allNodeKinds[nodeKind].funSemanticCheck;
}

FComputeType Nest_getComputeTypeFun(int nodeKind) {
    ASSERT(0 <= nodeKind && nodeKind < _numNodeKinds);
    return _allNodeKinds[nodeKind].funComputeType;
}

FSetContextForChildren Nest_getSetContextForChildrenFun(int nodeKind) {
    ASSERT(0 <= nodeKind && nodeKind < _numNodeKinds);
    return _allNodeKinds[nodeKind].funSetContextForChildren;
}

FToString Nest_getToStringFun(int nodeKind) {
    ASSERT(0 <= nodeKind && nodeKind < _numNodeKinds);
    return _allNodeKinds[nodeKind].funToString;
}
