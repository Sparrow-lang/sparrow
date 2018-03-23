#include "Nest/Api/NodeProperties.h"

#include <stdlib.h>

void Nest_addProperty(NodeProperties* properties, NodeProperty prop) {
    // Make sure we have enough space in our array
    unsigned curSize = properties->end - properties->begin;
    unsigned capacity = properties->endOfStore - properties->begin;
    if (capacity == 0)
        capacity = 8; // initial capacity
    while (capacity < curSize + 1)
        capacity += (capacity + 1) / 2; // growth factor: 1.5
    if (capacity > (properties->endOfStore - properties->begin)) {
        // Not enough space. Realloc the array
        unsigned curSize = properties->end - properties->begin;
        properties->begin = realloc(properties->begin, capacity * sizeof(NodeProperty));
        properties->end = properties->begin + curSize;
        properties->endOfStore = properties->begin + capacity;
    }
    // Actually add the node
    properties->begin[curSize] = prop;
    properties->end++;
}
