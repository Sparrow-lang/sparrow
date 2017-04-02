#pragma once

#include "Nest/Utils/NodeVector.hpp"
#include <SparrowFrontend/Nodes/Decls/AccessType.h>

namespace SprFrontend
{
    /// Get the declarations that the given node might refer to
    /// If the given node contains a "base expression" returns it in the last parameter
    NodeVector getDeclsFromNode(Node* n, Node*& baseExp);

    /// Get the resulting decls out of the given decls.
    ///
    /// A decl can be a using that points to other decls. Make sure we get the
    /// target decls.
    NodeArray expandDecls(NodeRange decls, Node* seenFrom);

    /// Get the resulting declaration from the given node
    Node* resultingDecl(Node* node);

    /// Check if the given node is a field or not
    bool isField(Node* node);


    /// Checks if we can access 'node' from 'fromNode'
    bool canAccessNode(Node* decl, Node* fromNode);
    /// Checks if we can access 'node' from the given source code
    bool canAccessNode(Node* decl, SourceCode* fromSourceCode);

    /// Check if the given node can be accessed from everywhere
    bool isPublic(Node* decl);


    /// Getter for the access type of the given node
    /// Returns the default access if the node doesn't have an explicit access type
    AccessType getAccessType(Node* decl);

    /// Tests if the given node has an access type set
    bool hasAccessType(Node* decl);

    /// Set the access type for a node
    void setAccessType(Node* decl, AccessType accessType);

    /// Deduce and set the access type for a node
    /// If the name of the node starts with '_', we consider private, otherwise public
    void deduceAccessType(Node* decl);

    /// Copy the access type from one declaration to the other
    void copyAccessType(Node* destDecl, Node* srcDecl);


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

    /// Check if the given function node has an implicit this parameter
    /// returns false if the given node is null or is not a function
    bool funHasImplicitThis(Node* fun);

    //! Returns the compilation context surrounding the given class
    //! If the context is not introduced by a node, move up to the first context
    //! introduced by a node, which is not a class-like node
    //! Case treated: If the class is introduced by a generic, it will search
    //! the context of the generic; if we have nested classes, get up to top-level scope
    CompilationContext* classContext(Node* cls);

    //! Get the decls with the given name associated with the given class
    NodeArray getClassAssociatedDecls(Node* cls, const char* name);
}
