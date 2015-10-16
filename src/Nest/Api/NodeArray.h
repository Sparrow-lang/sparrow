#pragma once

typedef struct Nest_Node Node;

/// An array of nodes
struct Nest_NodeArray {
    Node** beginPtr;
    Node** endPtr;
    Node** endOfStorePtr;
};
typedef struct Nest_NodeArray Nest_NodeArray;
typedef struct Nest_NodeArray NodeArray;

