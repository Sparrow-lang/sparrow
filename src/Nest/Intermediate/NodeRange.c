#include "NodeRange.h"

unsigned int Nest_nodeRangeSize(NodeRange nodes) {
    return nodes.endPtr - nodes.beginPtr;
}

NodeRange Nest_NodeRagenFromCArray(Node** nodes, unsigned int count) {
    NodeRange res = { nodes, nodes+count };
    return res;
}
