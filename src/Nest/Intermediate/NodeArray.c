#include "NodeArray.h"
#include "Common/Alloc.h"
#include "Common/Assert.h"

#include <memory.h>

void _growNodeArray(NodeArray* arr, unsigned int minCapacity) {
    unsigned int capacity = Nest_nodeArrayCapacity(*arr);
    if ( capacity == 0 ) {
        capacity = 8;   // initial capacity
    }
    else {
        while ( capacity < minCapacity )
            capacity += (capacity+1)/2;     // growth factor: 1.5
    }
    Nest_reserveNodeArray(arr, capacity);
}

NodeArray Nest_allocNodeArray(unsigned int capacity) {
    NodeArray res = { 0, 0, 0 };
    if ( capacity > 0 ) {
        res.begin = alloc(capacity* sizeof(Node*), allocGeneral);
        res.end = res.begin;
        res.endOfStore = res.begin + capacity;
    }
    return res;
}

void Nest_reserveNodeArray(NodeArray* arr, unsigned int capacity) {
    if ( capacity > Nest_nodeArrayCapacity(*arr) ) {
        // Allocate a new array
        NodeArray newArr = { 0, 0, 0 };
        newArr.begin = alloc(capacity* sizeof(Node*), allocGeneral);
        newArr.end = newArr.begin;
        newArr.endOfStore = newArr.begin + capacity;

        // Copy the elements to the new array
        memcpy(newArr.begin, arr->begin, Nest_nodeArraySize(*arr)*sizeof(Node*));

        // Change the pointers in the given array
        *arr = newArr;
    }
}

void Nest_resizeNodeArray(NodeArray* arr, unsigned int size) {
    unsigned int curSize = Nest_nodeArraySize(*arr);
    if ( size < curSize ) {
        // Simply change the end pointer
        arr->end = arr->begin + size;
    }
    else if ( size > curSize ) {
        // Make sure we have enough elements
        _growNodeArray(arr, size);

        // Then change the size
        arr->end = arr->begin + size;
    }
}

void Nest_appendNodeToArray(NodeArray* arr, Node* node) {
    unsigned int curSize = Nest_nodeArraySize(*arr);
    _growNodeArray(arr, curSize+1);
    arr->begin[curSize] = node;
}

void Nest_appendNodesToArray(NodeArray* arr, NodeRange nodes) {
    unsigned int numNewNodes = Nest_nodeRangeSize(nodes);
    unsigned int curSize = Nest_nodeArraySize(*arr);
    _growNodeArray(arr, curSize+numNewNodes);
    for ( unsigned int i=0; i<numNewNodes; ++i ) {    
        arr->begin[curSize+i] = nodes.begin[i];
    }
}

void Nest_insertNodeIntoArray(NodeArray* arr, unsigned int index, Node* node) {
    unsigned int curSize = Nest_nodeArraySize(*arr);
    ASSERT(index < curSize);
    _growNodeArray(arr, curSize+1);
    memmove(arr->begin+index+1, arr->begin+index, sizeof(Node*)*(curSize-index));
    arr->begin[index] = node;
}
void Nest_insertNodesIntoArray(NodeArray* arr, unsigned int index, NodeRange nodes) {
    unsigned int numNewNodes = Nest_nodeRangeSize(nodes);
    unsigned int curSize = Nest_nodeArraySize(*arr);
    ASSERT(index < curSize);
    _growNodeArray(arr, curSize+numNewNodes);
    memmove(arr->begin+index+numNewNodes, arr->begin+index, sizeof(Node*)*(curSize-index));
    for ( unsigned int i=0; i<numNewNodes; ++i ) {    
        arr->begin[index+i] = nodes.begin[i];
    }
}

NodeRange Nest_getNodeRangeFromArray(NodeArray arr) {
    NodeRange res = { arr.begin, arr.end };
    return res;
}

unsigned int Nest_nodeArraySize(NodeArray arr) {
    return arr.end - arr.begin;
}
unsigned int Nest_nodeArrayCapacity(NodeArray arr) {
    return arr.endOfStore - arr.begin;
}

