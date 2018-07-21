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
const char* Nest_toString(Nest_Node* node);

/// Returns a more detailed string description out of the given node
const char* Nest_toStringEx(Nest_Node* node);

/// Getter for the name of the node kind for the given node
const char* Nest_nodeKindName(Nest_Node* node);

/// Getter for the children of a node
Nest_NodeRange Nest_nodeChildren(Nest_Node* node);

/// Getter for the particular child node at a particular index of the given node,
Nest_Node* Nest_getNodeChild(Nest_Node* node, unsigned int index);

/// Getter for the list of nodes referred by a node
Nest_NodeRange Nest_nodeReferredNodes(Nest_Node* node);

/// Getter for the particular referred node at a particular index of the given node,
Nest_Node* Nest_getReferredNode(Nest_Node* node, unsigned int index);

/// Sets the children of the given node
void Nest_nodeSetChildren(Nest_Node* node, Nest_NodeRange children);

/// Adds a new node as a child to the given node
void Nest_nodeAddChild(Nest_Node* node, Nest_Node* child);

/// Adds a node range as children to the given node
void Nest_nodeAddChildren(Nest_Node* node, Nest_NodeRange children);

/// Sets the referred nodes of the given node
void Nest_nodeSetReferredNodes(Nest_Node* node, Nest_NodeRange nodes);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Node properties
//

void Nest_setProperty(Nest_Node* node, Nest_NodeProperty prop);
void Nest_setPropertyInt(Nest_Node* node, const char* name, int val);
void Nest_setPropertyString(Nest_Node* node, const char* name, Nest_StringRef val);
void Nest_setPropertyNode(Nest_Node* node, const char* name, Nest_Node* val);
void Nest_setPropertyType(Nest_Node* node, const char* name, Nest_TypeRef val);
void Nest_setPropertyPtr(Nest_Node* node, const char* name, void* val);

void Nest_setPropertyExplInt(Nest_Node* node, const char* name, int val);
void Nest_setPropertyExplString(Nest_Node* node, const char* name, Nest_StringRef val);
void Nest_setPropertyExplNode(Nest_Node* node, const char* name, Nest_Node* val);
void Nest_setPropertyExplType(Nest_Node* node, const char* name, Nest_TypeRef val);
void Nest_setPropertyExplPtr(Nest_Node* node, const char* name, void* val);

int Nest_hasProperty(Nest_Node* node, const char* name);
const int* Nest_getPropertyInt(Nest_Node* node, const char* name);
const Nest_StringRef* Nest_getPropertyString(Nest_Node* node, const char* name);
Nest_StringRef Nest_getPropertyStringDeref(Nest_Node* node, const char* name);
Nest_Node* const* Nest_getPropertyNode(Nest_Node* node, const char* name);
const Nest_TypeRef* Nest_getPropertyType(Nest_Node* node, const char* name);
void* const* Nest_getPropertyPtr(Nest_Node* node, const char* name);

int Nest_getPropertyDefaultInt(Nest_Node* node, const char* name, int defaultVal);
Nest_StringRef Nest_getPropertyDefaultString(Nest_Node* node, const char* name, Nest_StringRef defaultVal);
Nest_Node* Nest_getPropertyDefaultNode(Nest_Node* node, const char* name, Nest_Node* defaultVal);
Nest_TypeRef Nest_getPropertyDefaultType(Nest_Node* node, const char* name, Nest_TypeRef defaultVal);
void* Nest_getPropertyDefaultPtr(Nest_Node* node, const char* name, void* defaultVal);

int Nest_getCheckPropertyInt(Nest_Node* node, const char* name);
Nest_StringRef Nest_getCheckPropertyString(Nest_Node* node, const char* name);
Nest_Node* Nest_getCheckPropertyNode(Nest_Node* node, const char* name);
Nest_TypeRef Nest_getCheckPropertyType(Nest_Node* node, const char* name);
void* Nest_getCheckPropertyPtr(Nest_Node* node, const char* name);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Compilation processes
//

/// Add a modifier to this class; this modifier will be called before and after compilation
void Nest_addModifier(Nest_Node* node, Nest_Modifier* mod);

/// Returns the compilation context for the children of this node
Nest_CompilationContext* Nest_childrenContext(Nest_Node* node);

/// Getter for the explanation of this node, if it has one; otherwise returns this node
Nest_Node* Nest_explanation(Nest_Node* node);

/// Returns 'src' if the given node is of the specified type; null otherwise
Nest_Node* Nest_ofKind(Nest_Node* src, int desiredNodeKind);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Node array
//

Nest_NodeArray Nest_allocNodeArray(unsigned capacity);
void Nest_freeNodeArray(Nest_NodeArray arr);
void Nest_reserveNodeArray(Nest_NodeArray* arr, unsigned capacity);
void Nest_resizeNodeArray(Nest_NodeArray* arr, unsigned size);

void Nest_appendNodeToArray(Nest_NodeArray* arr, Nest_Node* node);
void Nest_appendNodesToArray(Nest_NodeArray* arr, Nest_NodeRange nodes);

void Nest_insertNodeIntoArray(Nest_NodeArray* arr, unsigned index, Nest_Node* node);
void Nest_insertNodesIntoArray(Nest_NodeArray* arr, unsigned index, Nest_NodeRange nodes);

void Nest_eraseNodeFromArray(Nest_NodeArray* arr, unsigned index);

Nest_NodeRange Nest_getNodeRangeFromArray(Nest_NodeArray arr);

unsigned Nest_nodeArraySize(Nest_NodeArray arr);
unsigned Nest_nodeArrayCapacity(Nest_NodeArray arr);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Node range
//

unsigned Nest_nodeRangeSize(Nest_NodeRange nodes);
unsigned Nest_nodeRangeMSize(Nest_NodeRangeM nodes);

Nest_NodeRange Nest_NodeRagenFromCArray(Nest_Node** nodes, unsigned count);

#ifdef __cplusplus
}
#endif
