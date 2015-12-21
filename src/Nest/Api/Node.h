#pragma once

#include "Nest/Api/NodeArray.h"
#include "Nest/Api/Modifier.h"
#include "Nest/Api/TypeRef.h"
#include "Nest/Api/NodeProperties.h"
#include "Nest/Api/Location.h"
#include "Nest/Api/StringRef.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Nest_Node Node;
typedef struct Nest_Modifier Modifier;
typedef struct Nest_CompilationContext CompilationContext;

/// Structure representing an AST node
struct Nest_Node
{
    /// Node flags
    unsigned int nodeKind : 16;
    unsigned int nodeError : 1;
    unsigned int nodeSemanticallyChecked : 1;
    unsigned int computeTypeStarted : 1;
    unsigned int semanticCheckStarted : 1;

    /// The location corresponding to this node
    Location location;

    /// The children of this node
    NodeArray children;

    /// The nodes referred by this node
    NodeArray referredNodes;
    
    /// The properties of the node
    NodeProperties properties;

    /// The context of this node
    CompilationContext* context;
    
    /// The context of the children
    CompilationContext* childrenContext;
    
    /// The type of this node
    TypeRef type;
    
    /// The explanation of this node
    /// A node has explanation if its meaning can be explained with the help of another node (the explanation node)
    Node* explanation;

    /// The modifiers used to adjust the compilation process of this node
    ModifiersArray modifiers;
};

typedef struct Nest_Node Nest_Node;
typedef struct Nest_Node Node;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// General Node operations
//

/// Creates a node of the given kind
Node* Nest_createNode(int nodeKind);

/// Clone the given node - create one with the same characteristics
/// We clear the compilation state of the new node when cloning
Node* Nest_cloneNode(Node* node);


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Compilation processes
//

/// Sets the compilation context for this node; with this we know where this node is put in the program
/// This operation always succeeds
/// Setting the context for the children may fail; the error needs to be handled by the user
void Nest_setContext(Node* node, CompilationContext* context);

/// Just computes the type, without performing all the semantic check actions; used for declarations
/// Returns the type if succeeded, NULL if failure
TypeRef Nest_computeType(Node* node);

/// Performs all the semantic-check actions
/// Returns the explanation if succeeded, NULL if failure
Node* Nest_semanticCheck(Node* node);

/// Reverts the compilation state; brings the node to un-compiled state
void Nest_clearCompilationState(Node* node);


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Default implementation for node-specific functions
//

/// Returns a string description of the given node
const char* Nest_defaultFunToString(const Node* node);

/// Sets the context for all the children of the given node
void Nest_defaultFunSetContextForChildren(Node* node);

/// Computes the type of the node - calls semantic check to compute the type
TypeRef Nest_defaultFunComputeType(Node* node);

#ifdef __cplusplus
}
#endif
