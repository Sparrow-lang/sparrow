#pragma once

#include <Feather/Nodes/NodeList.h>

namespace SprFrontend
{
    /// Get the declarations that the given node might refer to
    /// If the given node contains a "base expression" returns it in the last parameter
    DynNodeVector getDeclsFromNode(DynNode* n, DynNode*& baseExp);

    /// Get the resulting declaration from the given node
    DynNode* resultingDecl(DynNode* node);
    
    /// Check if the given node is a field or not
    bool isField(DynNode* node);


    /// Getter for the access type of the given node
    /// Throws if the node doesn't have an associated access type
    AccessType getAccessType(DynNode* decl);

    /// Tests if the given node has an access type set
    bool hasAccessType(DynNode* decl);

    /// Set the access type for a node
    void setAccessType(DynNode* decl, AccessType accessType);
    
    
    /// Gets the result parameter of a function, if it has one
    DynNode* getResultParam(DynNode* f);
    
    /// Checks all the children of a namespace or a class are of allowed types
    void checkForAllowedNamespaceChildren(NodeList* children, bool insideClass = false);

    /// Copy the modifiers from the source node to the destination node; we do not copy any mode changing modifiers;
    /// instead we add a new modifier to change the mode
    void copyModifiersSetMode(DynNode* src, DynNode* dest, EvalMode newMode);
}
