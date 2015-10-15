#pragma once

#include "NodeRange.h"

#ifdef __cplusplus
extern "C" {
#endif

struct Nest_NodeArray {
    Node** beginPtr;
    Node** endPtr;
    Node** endOfStorePtr;
};
typedef struct Nest_NodeArray Nest_NodeArray;
typedef struct Nest_NodeArray NodeArray;


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

#ifdef __cplusplus
}
#endif

