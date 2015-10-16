#pragma once

#include "Nest/Api/Node.h"
#include "Nest/Api/NodeRange.h"

#ifdef __cplusplus
extern "C" {
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// General Node operations
//

/// Returns a string description out of the given node
const char* Nest_toString(const Node* node);

/// Getter for the name of the node kind for the given node
const char* Nest_nodeKindName(const Node* node);

/// Getter for the children of a node
NodeRange Nest_nodeChildren(Node* node);

/// Getter for the particular child node at a particular index of the given node,
Node* Nest_getNodeChild(Node* node, unsigned int index);

/// Getter for the list of nodes referred by a node
NodeRange Nest_nodeReferredNodes(Node* node);

/// Getter for the particular referred node at a particular index of the given node,
Node* Nest_getReferredNode(Node* node, unsigned int index);

/// Sets the children of the given node
void Nest_nodeSetChildren(Node* node, NodeRange children);

/// Adds a new node as a child to the given node
void Nest_nodeAddChild(Node* node, Node* child);

/// Adds a node range as children to the given node
void Nest_nodeAddChildren(Node* node, NodeRange children);

/// Sets the referred nodes of the given node
void Nest_nodeSetReferredNodes(Node* node, NodeRange nodes);


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Node properties
//

void Nest_setProperty(Node* node, NodeProperty prop);
void Nest_setPropertyInt(Node* node, const char* name, int val);
void Nest_setPropertyString(Node* node, const char* name, StringRef val);
void Nest_setPropertyNode(Node* node, const char* name, Node* val);
void Nest_setPropertyType(Node* node, const char* name, TypeRef val);

void Nest_setPropertyExplInt(Node* node, const char* name, int val);
void Nest_setPropertyExplString(Node* node, const char* name, StringRef val);
void Nest_setPropertyExplNode(Node* node, const char* name, Node* val);
void Nest_setPropertyExplType(Node* node, const char* name, TypeRef val);

int Nest_hasProperty(const Node* node, const char* name);
const int* Nest_getPropertyInt(const Node* node, const char* name);
const StringRef* Nest_getPropertyString(const Node* node, const char* name);
Node*const* Nest_getPropertyNode(const Node* node, const char* name);
const TypeRef* Nest_getPropertyType(const Node* node, const char* name);

int Nest_getCheckPropertyInt(const Node* node, const char* name);
StringRef Nest_getCheckPropertyString(const Node* node, const char* name);
Node* Nest_getCheckPropertyNode(const Node* node, const char* name);
TypeRef Nest_getCheckPropertyType(const Node* node, const char* name);


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Compilation processes
//

/// Add a modifier to this class; this modifier will be called before and after compilation
void Nest_addModifier(Node* node, Modifier* mod);

/// Returns the compilation context for the children of this node
CompilationContext* Nest_childrenContext(const Node* node);

/// Getter for the explanation of this node, if it has one; otherwise returns this node
Node* Nest_explanation(Node* node);

/// Returns 'src' if the given node is of the specified type; null otherwise
Node* Nest_ofKind(Node* src, int desiredNodeKind);


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Node array
//

NodeArray Nest_allocNodeArray(unsigned capacity);
void Nest_freeNodeArray(NodeArray arr);
void Nest_reserveNodeArray(NodeArray* arr, unsigned capacity);
void Nest_resizeNodeArray(NodeArray* arr, unsigned size);

void Nest_appendNodeToArray(NodeArray* arr, Node* node);
void Nest_appendNodesToArray(NodeArray* arr, NodeRange nodes);

void Nest_insertNodeIntoArray(NodeArray* arr, unsigned index, Node* node);
void Nest_insertNodesIntoArray(NodeArray* arr, unsigned index, NodeRange nodes);

void Nest_eraseNodeFromArray(NodeArray* arr, unsigned index);

NodeRange Nest_getNodeRangeFromArray(NodeArray arr);

unsigned Nest_nodeArraySize(NodeArray arr);
unsigned Nest_nodeArrayCapacity(NodeArray arr);


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Node range
//

unsigned Nest_nodeRangeSize(NodeRange nodes);

NodeRange Nest_NodeRagenFromCArray(Node** nodes, unsigned count);

#ifdef __cplusplus
}
#endif
