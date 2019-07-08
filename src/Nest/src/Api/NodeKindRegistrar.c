#include "Nest/Api/NodeKindRegistrar.h"
#include "Nest/Api/Node.h"
#include "Nest/Utils/Assert.h"
#include "Nest/Utils/Profiling.h"

#include <stdlib.h>
#include <string.h>

struct _NodeKindDescription {
    const char* name;
    FSemanticCheck funSemanticCheck;
    FComputeType funComputeType;
    FSetContextForChildren funSetContextForChildren;
    FToString funToString;

#if SPARROW_PROFILING
    const Nest_Profiling_LocType* setContextLoc;
    const Nest_Profiling_LocType* computeTypeLoc;
    const Nest_Profiling_LocType* semanticCheckLoc;
#endif
};

#if SPARROW_PROFILING
const char* strconcat(const char* s1, const char* s2) {
    size_t len = strlen(s1) + strlen(s2) + 1;
    char* res = (char*)malloc(len);
    res[0] = 0;
    strcat(res, s1);
    strcat(res, s2);
    return res;
}
#endif

/// The registered node kinds
struct _NodeKindDescription _allNodeKinds[1000];
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

#if SPARROW_PROFILING
    const Nest_Profiling_LocType* setContextLoc = Nest_Profiling_createLoc(
            strconcat("setCtx ", name), "Nest_setContext", "Node.c", 1, 0x4f94cd); // SteelBlue3
    const Nest_Profiling_LocType* computeTypeLoc = Nest_Profiling_createLoc(
            strconcat("compType ", name), "Nest_computeType", "Node.c", 1, 0x36648b); // SteelBlue4
    const Nest_Profiling_LocType* semanticCheckLoc =
            Nest_Profiling_createLoc(strconcat("semCheck ", name), "Nest_semanticCheck", "Node.c",
                    1, 0x27408b); // RoyalBlue4
#endif

    struct _NodeKindDescription nk = {
        name,
        funSemanticCheck,
        funComputeType,
        funSetContextForChildren,
        funToString,
#if SPARROW_PROFILING
        setContextLoc,
        computeTypeLoc,
        semanticCheckLoc,
#endif
    };
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

void Nest_resetRegisteredNodeKinds() { _numNodeKinds = 0; }

#if SPARROW_PROFILING

const Nest_Profiling_LocType* Nest_Profiling_getSetContextLoc(int nodeKind) {
    return _allNodeKinds[nodeKind].setContextLoc;
}
const Nest_Profiling_LocType* Nest_Profiling_getComputeTypeLoc(int nodeKind) {
    return _allNodeKinds[nodeKind].computeTypeLoc;
}
const Nest_Profiling_LocType* Nest_Profiling_getSemanticCheckLoc(int nodeKind) {
    return _allNodeKinds[nodeKind].semanticCheckLoc;
}

#endif
