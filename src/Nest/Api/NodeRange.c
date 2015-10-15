#include "NodeRange.h"

unsigned Nest_nodeRangeSize(NodeRange nodes) {
    return nodes.endPtr - nodes.beginPtr;
}

NodeRange Nest_NodeRagenFromCArray(Node** nodes, unsigned count) {
    NodeRange res = { nodes, nodes+count };
    return res;
}
