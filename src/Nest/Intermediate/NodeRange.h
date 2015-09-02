#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Nest_Node Node;

struct Nest_NodeRange {
    Node** begin;
    Node** end;
};
typedef struct Nest_NodeRange Nest_NodeRange;
typedef struct Nest_NodeRange NodeRange;

unsigned int Nest_nodeRangeSize(NodeRange nodes);

#ifdef __cplusplus
}
#endif

