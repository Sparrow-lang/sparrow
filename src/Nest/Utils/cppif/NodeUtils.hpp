#pragma once

#include "Nest/Utils/cppif/Fwd.hpp"
#include "Nest/Utils/NodeUtils.h"

namespace Nest {

using NodeVector = vector<Node*>;

inline Nest_NodeRange all(Nest_NodeArray nodes) { return {nodes.beginPtr, nodes.endPtr}; }
inline Nest_NodeRangeM allM(Nest_NodeArray nodes) { return {nodes.beginPtr, nodes.endPtr}; }

inline Nest_Node* const* begin(Nest_NodeRange r) { return r.beginPtr; }
inline Nest_Node* const* end(Nest_NodeRange r) { return r.endPtr; }

inline Nest_Node** begin(Nest_NodeRangeM r) { return r.beginPtr; }
inline Nest_Node** end(Nest_NodeRangeM r) { return r.endPtr; }

inline Nest_Node** begin(Nest_NodeArray r) { return r.beginPtr; }
inline Nest_Node** end(Nest_NodeArray r) { return r.endPtr; }

/// Construct a Nest_NodeRange from an initializer_list
inline Nest_NodeRange fromIniList(std::initializer_list<Nest_Node*> l) {
    Nest_NodeRange res = {l.begin(), l.end()};
    return res;
}

inline Nest_Node* at(Nest_NodeRange r, unsigned int idx) { return r.beginPtr[idx]; }
inline Nest_Node*& at(Nest_NodeRangeM r, unsigned int idx) { return r.beginPtr[idx]; }

inline Nest_Node*& at(Nest_NodeArray arr, unsigned int idx) { return arr.beginPtr[idx]; }

inline unsigned int size(Nest_NodeRange r) { return Nest_nodeRangeSize(r); }
inline unsigned int size(Nest_NodeRangeM r) { return Nest_nodeRangeMSize(r); }
inline unsigned int size(Nest_NodeArray arr) { return Nest_nodeArraySize(arr); }

inline Nest_NodeRange all(const NodeVector& nodes) {
    Nest_NodeRange res = {&*nodes.begin(), &*nodes.end()};
    return res;
}
inline Nest_NodeRange subrange(NodeVector& nodes, int startIdx, int endIdx) {
    Nest_NodeRange res = {&*(nodes.begin() + startIdx), &*(nodes.begin() + endIdx)};
    return res;
}

inline Nest_NodeRange subrange(Nest_NodeRange nodes, int startIdx, int endIdx) {
    Nest_NodeRange res = {nodes.beginPtr + startIdx, nodes.beginPtr + endIdx};
    return res;
}

inline NodeVector toVec(Nest_NodeArray nodes) { return NodeVector(nodes.beginPtr, nodes.endPtr); }
inline NodeVector toVec(Nest_NodeRange nodes) { return NodeVector(nodes.beginPtr, nodes.endPtr); }

} // namespace Nest
