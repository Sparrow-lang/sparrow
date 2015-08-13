#pragma once

namespace SprFrontend
{
    /// Get the declarations that the given node might refer to
    /// If the given node contains a "base expression" returns it in the last parameter
    NodeVector getDeclsFromNode(Node* n, Node*& baseExp);

    /// Get the resulting declaration from the given node
    Node* resultingDecl(Node* node);
    
    /// Check if the given node is a field or not
    bool isField(Node* node);


    /// Getter for the access type of the given node
    /// Throws if the node doesn't have an associated access type
    AccessType getAccessType(Node* decl);

    /// Tests if the given node has an access type set
    bool hasAccessType(Node* decl);

    /// Set the access type for a node
    void setAccessType(Node* decl, AccessType accessType);
    
    
    /// Gets the result parameter of a function, if it has one
    Node* getResultParam(Node* f);
    
    /// Checks all the children of a namespace or a class are of allowed types
    void checkForAllowedNamespaceChildren(Node* children, bool insideClass = false);

    /// Copy the modifiers from the source node to the destination node; we do not copy any mode changing modifiers;
    /// instead we add a new modifier to change the mode
    void copyModifiersSetMode(Node* src, Node* dest, EvalMode newMode);

    /// Check if the given function node has a this parameter
    /// returns false if the given node is null or is not a function
    bool funHasThisParameters(Node* fun);
}
