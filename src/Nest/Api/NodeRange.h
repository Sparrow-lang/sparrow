#pragma once

typedef struct Nest_Node Node;

struct Nest_NodeRange {
    Node** beginPtr;
    Node** endPtr;
};
typedef struct Nest_NodeRange Nest_NodeRange;
typedef struct Nest_NodeRange NodeRange;
