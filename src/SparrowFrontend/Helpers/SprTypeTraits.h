#pragma once

#include <NodeCommonsH.h>

#include "Feather/Utils/FeatherUtils.hpp"

namespace SprFrontend {
/// Gets the common type between two types; Void is used if the types are irrelated
TypeRef commonType(CompilationContext* context, TypeRef t1, TypeRef t2);

/// If the given argument if of type 'ref ref ... ref T', returns T and gets the conversion need to
/// access 'ref T'
TypeRef doDereference1(Node* arg, Node*& cvt);

/// Converts a CT-available node into a node that can be directly used at RT - usually invoked by
/// the backend when translating to RT
Node* convertCtToRt(Node* node);

/// Given a node that represents a type, get the type expressed by the given node
/// Yields an error if the given node doesn't indicate to a type
TypeRef getType(Node* typeNode);

/// Given a node representing a 'Type' value, get its type.
/// Returns null if cannot evaluate node
TypeRef tryGetTypeValue(Node* typeNode);

/// If this is a type that represents another type, evaluate this it to get the inner type.
/// Otherwise return the original type
TypeRef evalTypeIfPossible(Node* typeNode);

/// create a type node from the given type
Node* createTypeNode(CompilationContext* context, const Location& loc, TypeRef t);

/// Gets the type to be used when auto is found for a node; removes l-values, but tries to preserve
/// references
TypeRef getAutoType(Node* typeNode, bool Feather_addRef = false, EvalMode evalMode = modeRtCt);

/// Tests if this an concept or concept-ref type
bool isConceptType(TypeRef t);
bool isConceptType(TypeRef t, bool& isRefAuto);

/// Creates a new type from the original type, with the specified reference count
TypeRef changeRefCount(TypeRef type, int numRef, const Location& loc = Location());
} // namespace SprFrontend