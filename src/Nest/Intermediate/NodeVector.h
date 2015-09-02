#pragma once

#include "NodeArray.h"
#include "NodeRange.h"

typedef struct Nest_Node Node;
typedef vector<Node*> NodeVector;

inline NodeRange all(NodeVector& nodes)
{
    NodeRange res = { &*nodes.begin(), &*nodes.end() };
    return res;
}

