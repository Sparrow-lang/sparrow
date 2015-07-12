#pragma once

#include <NodeCommonsH.h>

#include <Feather/Util/TypeTraits.h>

namespace SprFrontend
{
    /// Gets the common type between two types; Void is used if the types are irrelated
    Nest::TypeRef commonType(CompilationContext* context, Nest::TypeRef t1, Nest::TypeRef t2);
    
    /// If the given argument if of type 'ref ref ... ref T', returns T and gets the conversion need to access 'ref T'
    Nest::TypeRef doDereference1(Nest::DynNode* arg, Nest::DynNode*& cvt);
    
    /// Converts a CT-available node into a node that can be directly used at RT - usually invoked by the backend when translating to RT
    Node* convertCtToRt(Node* node);

    /// Given a node that represents a type, get the type expressed by the given node
    /// Yields an error if the given node doesn't indicate to a type
    Nest::TypeRef getType(Nest::DynNode* typeNode);
    
    /// Given a node representing a 'Type' value, get its type.
    /// Returns null if cannot evaluate node
    Nest::TypeRef tryGetTypeValue(Nest::DynNode* typeNode);
    
    /// If this is a type that represents another type, evaluate this it to get the inner type.
    /// Otherwise return the original type
    Nest::TypeRef evalTypeIfPossible(Nest::DynNode* typeNode);
    
    /// create a type node from the given type
    Nest::DynNode* createTypeNode(CompilationContext* context, const Nest::Location& loc, Nest::TypeRef t);
    
    
    /// Gets the type to be used when auto is found for a node; removes l-values, but tries to preserve references
    Nest::TypeRef getAutoType(Nest::DynNode* typeNode, bool addRef = false);
    
    /// Tests if this an concept or concept-ref type
    bool isConceptType(Nest::TypeRef t);
    bool isConceptType(Nest::TypeRef t, bool& isRefAuto);


    /// Creates a new type from the original type, with the specified reference count
    Nest::TypeRef changeRefCount(Nest::TypeRef type, int numRef, const Nest::Location& loc = Nest::Location());
}