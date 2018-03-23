#pragma once

typedef struct Nest_Node Node;

struct Nest_NodeRange {
    Node* const* beginPtr;
    Node* const* endPtr;
};
typedef struct Nest_NodeRange Nest_NodeRange;
typedef struct Nest_NodeRange NodeRange;

struct Nest_NodeRangeM {
    Node** beginPtr;
    Node** endPtr;
};
typedef struct Nest_NodeRangeM Nest_NodeRangeM;
typedef struct Nest_NodeRangeM NodeRangeM;
