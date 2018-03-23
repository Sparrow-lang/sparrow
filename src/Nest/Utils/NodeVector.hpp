#pragma once

#include "NodeUtils.hpp"

using Node = struct Nest_Node;
using NodeVector = vector<Node*>;

inline NodeRange all(NodeVector& nodes) {
    NodeRange res = {&*nodes.begin(), &*nodes.end()};
    return res;
}
inline NodeRange subrange(NodeVector& nodes, int startIdx, int endIdx) {
    NodeRange res = {&*(nodes.begin() + startIdx), &*(nodes.begin() + endIdx)};
    return res;
}

inline NodeRange subrange(NodeRange nodes, int startIdx, int endIdx) {
    NodeRange res = {nodes.beginPtr + startIdx, nodes.beginPtr + endIdx};
    return res;
}

inline NodeVector toVec(NodeArray nodes) { return NodeVector(nodes.beginPtr, nodes.endPtr); }
inline NodeVector toVec(NodeRange nodes) { return NodeVector(nodes.beginPtr, nodes.endPtr); }
