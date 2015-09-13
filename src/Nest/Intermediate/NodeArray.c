#include "NodeArray.h"
#include "Common/Alloc.h"
#include "Common/Assert.h"

#include <memory.h>

void _growNodeArray(NodeArray* arr, unsigned int minCapacity) {
    unsigned int capacity = Nest_nodeArrayCapacity(*arr);
    if ( capacity == 0 )
        capacity = 8;   // initial capacity
    while ( capacity < minCapacity )
        capacity += (capacity+1)/2;     // growth factor: 1.5
    Nest_reserveNodeArray(arr, capacity);
}

NodeArray Nest_allocNodeArray(unsigned int capacity) {
    NodeArray res = { 0, 0, 0 };
    if ( capacity > 0 ) {
        res.beginPtr = alloc(capacity* sizeof(Node*), allocGeneral);
        res.endPtr = res.beginPtr;
        res.endOfStorePtr = res.beginPtr + capacity;
    }
    return res;
}

void Nest_freeNodeArray(NodeArray arr) {
    // TODO (memory): if this was the last thing allocated, clean it up
}


void Nest_reserveNodeArray(NodeArray* arr, unsigned int capacity) {
    if ( capacity > Nest_nodeArrayCapacity(*arr) ) {
        // Allocate a new array
        NodeArray newArr = Nest_allocNodeArray(capacity);

        // Copy the elements to the new array
        unsigned size = Nest_nodeArraySize(*arr);
        memcpy(newArr.beginPtr, arr->beginPtr, size*sizeof(Node*));
        newArr.endPtr = newArr.beginPtr + size;

        // Change the pointers in the given array
        *arr = newArr;
    }
}

void Nest_resizeNodeArray(NodeArray* arr, unsigned int size) {
    unsigned int curSize = Nest_nodeArraySize(*arr);
    if ( size < curSize ) {
        // Simply change the end pointer
        arr->endPtr = arr->beginPtr + size;
    }
    else if ( size > curSize ) {
        // Make sure we have enough elements
        _growNodeArray(arr, size);

        // Then change the size
        arr->endPtr = arr->beginPtr + size;
    }
}

void Nest_appendNodeToArray(NodeArray* arr, Node* node) {
    unsigned int curSize = Nest_nodeArraySize(*arr);
    _growNodeArray(arr, curSize+1);
    arr->beginPtr[curSize] = node;
    arr->endPtr++;
}

void Nest_appendNodesToArray(NodeArray* arr, NodeRange nodes) {
    unsigned int numNewNodes = Nest_nodeRangeSize(nodes);
    unsigned int curSize = Nest_nodeArraySize(*arr);
    _growNodeArray(arr, curSize+numNewNodes);
    for ( unsigned int i=0; i<numNewNodes; ++i ) {    
        arr->beginPtr[curSize+i] = nodes.beginPtr[i];
    }
    arr->endPtr += numNewNodes;
}

void Nest_insertNodeIntoArray(NodeArray* arr, unsigned int index, Node* node) {
    unsigned int curSize = Nest_nodeArraySize(*arr);
    ASSERT(index <= curSize);
    _growNodeArray(arr, curSize+1);
    memmove(arr->beginPtr+index+1, arr->beginPtr+index, sizeof(Node*)*(curSize-index));
    arr->beginPtr[index] = node;
    arr->endPtr++;
}
void Nest_insertNodesIntoArray(NodeArray* arr, unsigned int index, NodeRange nodes) {
    unsigned int numNewNodes = Nest_nodeRangeSize(nodes);
    unsigned int curSize = Nest_nodeArraySize(*arr);
    ASSERT(index <= curSize);
    _growNodeArray(arr, curSize+numNewNodes);
    memmove(arr->beginPtr+index+numNewNodes, arr->beginPtr+index, sizeof(Node*)*(curSize-index));
    for ( unsigned int i=0; i<numNewNodes; ++i ) {    
        arr->beginPtr[index+i] = nodes.beginPtr[i];
    }
    arr->endPtr += numNewNodes;
}

void Nest_eraseNodeFromArray(NodeArray* arr, unsigned int index) {
    unsigned int curSize = Nest_nodeArraySize(*arr);
    ASSERT(index <= curSize);
    memmove(arr->beginPtr+index, arr->beginPtr+index+1, sizeof(Node*)*(curSize-index-1));
    arr->endPtr--;
}


NodeRange Nest_getNodeRangeFromArray(NodeArray arr) {
    NodeRange res = { arr.beginPtr, arr.endPtr };
    return res;
}

unsigned int Nest_nodeArraySize(NodeArray arr) {
    return arr.endPtr - arr.beginPtr;
}
unsigned int Nest_nodeArrayCapacity(NodeArray arr) {
    return arr.endOfStorePtr - arr.beginPtr;
}

