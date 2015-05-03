#pragma once

#include <NodeCommonsH.h>

#include <Feather/Util/TypeTraits.h>

namespace SprFrontend
{
    /// Gets the common type between two types; Void is used if the types are irrelated
    Nest::Type* commonType(CompilationContext* context, Nest::Type* t1, Nest::Type* t2);
    
    /// If the given argument if of type 'ref ref ... ref T', returns T and gets the conversion need to access 'ref T'
    Nest::Type* doDereference1(Nest::Node* arg, Nest::Node*& cvt);
    
    /// Converts a CT-available node into a node that can be directly used at RT - usually invoked by the backend when translating to RT
    Node* convertCtToRt(Node* node);

    /// Given a node that represents a type, get the type expressed by the given node
    /// Yields an error if the given node doesn't indicate to a type
    Nest::Type* getType(Nest::Node* typeNode);
    
    /// Given a node representing a 'Type' value, get its type.
    /// Returns null if cannot evaluate node
    Nest::Type* tryGetTypeValue(Nest::Node* typeNode);
    
    /// If this is a type that represents another type, evaluate this it to get the inner type.
    /// Otherwise return the original type
    Nest::Type* evalTypeIfPossible(Nest::Node* typeNode);
    
    /// create a type node from the given type
    Nest::Node* createTypeNode(CompilationContext* context, const Nest::Location& loc, Nest::Type* t);
    
    
    /// Gets the type to be used when auto is found for a node; removes l-values, but tries to preserve references
    Nest::Type* getAutoType(Nest::Node* typeNode, bool addRef = false);
    
    /// Tests if this an concept or concept-ref type
    bool isConceptType(Nest::Type* t);
    bool isConceptType(Nest::Type* t, bool& isRefAuto);


    /// Creates a new type from the original type, with the specified reference count
    Nest::Type* changeRefCount(Nest::Type* type, int numRef, const Nest::Location& loc = Nest::Location());
}