#pragma once

#include "NodeUtils.hpp"

typedef struct Nest_Node Node;
typedef vector<Node*> NodeVector;


inline NodeRange all(NodeVector& nodes) {
    NodeRange res = { &*nodes.begin(), &*nodes.end() };
    return res;
}

inline NodeVector toVec(NodeArray nodes) {
    return NodeVector(nodes.beginPtr, nodes.endPtr);
}

