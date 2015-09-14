#include "NodeArray.h"
#include "Common/PtrArray.h"
#include "Common/Assert.h"

#include <memory.h>

NodeArray _fromPtrArray(PtrArray arr) {
    NodeArray res = { (Node**) arr.beginPtr, (Node**) arr.endPtr, (Node**) arr.endOfStorePtr };
    return res;
}
PtrArray _toPtrArray(NodeArray arr) {
    PtrArray res = { (void**) arr.beginPtr, (void**) arr.endPtr, (void**) arr.endOfStorePtr };
    return res;
}

NodeArray Nest_allocNodeArray(unsigned capacity) {
    return _fromPtrArray(NestUtils_allocPtrArray(capacity));
}

void Nest_freeNodeArray(NodeArray arr) {
    NestUtils_freePtrArray(_toPtrArray(arr));
}


void Nest_reserveNodeArray(NodeArray* arr, unsigned capacity) {
    PtrArray arr2 = _toPtrArray(*arr);
    NestUtils_reservePtrArray(&arr2, capacity);
    *arr = _fromPtrArray(arr2);
}

void Nest_resizeNodeArray(NodeArray* arr, unsigned size) {
    PtrArray arr2 = _toPtrArray(*arr);
    NestUtils_resizePtrArray(&arr2, size);
    *arr = _fromPtrArray(arr2);
}

void Nest_appendNodeToArray(NodeArray* arr, Node* node) {
    PtrArray arr2 = _toPtrArray(*arr);
    NestUtils_appendObjectToPtrArray(&arr2, node);
    *arr = _fromPtrArray(arr2);
}

void Nest_appendNodesToArray(NodeArray* arr, NodeRange nodes) {
    PtrArray arr2 = _toPtrArray(*arr);
    PtrRange objects = { (void**) nodes.beginPtr, (void**) nodes.endPtr };
    NestUtils_appendObjectsToPtrArray(&arr2, objects);
    *arr = _fromPtrArray(arr2);
}

void Nest_insertNodeIntoArray(NodeArray* arr, unsigned index, Node* node) {
    PtrArray arr2 = _toPtrArray(*arr);
    NestUtils_insertObjectIntoPtrArray(&arr2, index, node);
    *arr = _fromPtrArray(arr2);
}
void Nest_insertNodesIntoArray(NodeArray* arr, unsigned index, NodeRange nodes) {
    PtrArray arr2 = _toPtrArray(*arr);
    PtrRange objects = { (void**) nodes.beginPtr, (void**) nodes.endPtr };
    NestUtils_insertObjectsIntoPtrArray(&arr2, index, objects);
    *arr = _fromPtrArray(arr2);
}

void Nest_eraseNodeFromArray(NodeArray* arr, unsigned index) {
    PtrArray arr2 = _toPtrArray(*arr);
    NestUtils_eraseFromPtrArray(&arr2, index);
    *arr = _fromPtrArray(arr2);
}


NodeRange Nest_getNodeRangeFromArray(NodeArray arr) {
    NodeRange res = { arr.beginPtr, arr.endPtr };
    return res;
}

unsigned Nest_nodeArraySize(NodeArray arr) {
    return arr.endPtr - arr.beginPtr;
}
unsigned Nest_nodeArrayCapacity(NodeArray arr) {
    return arr.endOfStorePtr - arr.beginPtr;
}

