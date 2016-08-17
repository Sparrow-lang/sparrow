#pragma once

typedef struct Nest_Node Node;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Dumps AST starting at the given node to a .json file
 *
 * This will create a .json file describing the AST tree starting with the given
 * node. We include children, referred nodes and properties of all the nodes.
 *
 * If a node is serialized once, it will not be serialized a second time into
 * the same dump.
 * 
 * @param node     The start node to be dumped
 * @param filename The filename where to dump the AST
 */
void dumpAstNode(Node* node, const char* filename);

#ifdef __cplusplus
}
#endif

