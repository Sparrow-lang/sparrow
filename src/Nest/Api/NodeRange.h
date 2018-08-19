#pragma once

/**
 * @brief   A range of nodes (immutable).
 *
 * This represents a view on an array of nodes (pointers).
 * The nodes in the array cannot be changed through this. But the content of the nodes can be
 * mutated.
 */
struct Nest_NodeRange {
    struct Nest_Node* const* beginPtr;
    struct Nest_Node* const* endPtr;
};
typedef struct Nest_NodeRange Nest_NodeRange;

/**
 * @brief   A range of mutable nodes.
 *
 * This represents a view on an array of nodes (pointers).
 * The nodes in the array, and their pointers can be changed through this.
 */
struct Nest_NodeRangeM {
    struct Nest_Node** beginPtr;
    struct Nest_Node** endPtr;
};
typedef struct Nest_NodeRangeM Nest_NodeRangeM;
