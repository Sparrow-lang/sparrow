#pragma once

typedef struct Nest_Node Nest_Node;

/// An array of nodes
struct Nest_NodeArray {
    Nest_Node** beginPtr;
    Nest_Node** endPtr;
    Nest_Node** endOfStorePtr;
};
typedef struct Nest_NodeArray Nest_NodeArray;
