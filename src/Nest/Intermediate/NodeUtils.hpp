#pragma once

#include "NodeArray.h"
#include "NodeRange.h"

inline NodeRange all(NodeArray nodes) {
    NodeRange res = { nodes.beginPtr, nodes.endPtr };
    return res;
}

inline Node** begin(NodeRange r) { return r.beginPtr; }
inline Node** end(NodeRange r) { return r.endPtr; }

inline Node** begin(NodeArray r) { return r.beginPtr; }
inline Node** end(NodeArray r) { return r.endPtr; }

/// Construct a NodeRange from an initializer_list
inline NodeRange fromIniList(std::initializer_list<Node*> l) {
    NodeRange res = { (Node**) l.begin(), (Node**) l.end() };
    return res;
}

inline Node*& at(NodeRange r, unsigned int idx) {
    return r.beginPtr[idx];
}

inline Node*& at(NodeArray arr, unsigned int idx) {
    return arr.beginPtr[idx];
}
