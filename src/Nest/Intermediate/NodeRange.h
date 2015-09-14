#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Nest_Node Node;

struct Nest_NodeRange {
    Node** beginPtr;
    Node** endPtr;
};
typedef struct Nest_NodeRange Nest_NodeRange;
typedef struct Nest_NodeRange NodeRange;

unsigned Nest_nodeRangeSize(NodeRange nodes);

NodeRange Nest_NodeRagenFromCArray(Node** nodes, unsigned count);

#ifdef __cplusplus
}
#endif

