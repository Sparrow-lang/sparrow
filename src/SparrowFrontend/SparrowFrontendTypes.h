#pragma once

#include "Nest/Api/TypeRef.h"


namespace SprFrontend
{
    using namespace Nest;

    // The type kinds for the SparrowFrontend types
    extern int typeKindConcept;

    /// Called to initialize the SparrowFrontend type kinds
    void initSparrowFrontendTypeKinds();

    /// Returns a type that represents a concept type
    /// A type that represents a generic set of types, types that model a given concept. If no concept is given, the
    /// type will model any type
    TypeRef getConceptType(Node* concept = nullptr, uint8_t numReferences = 0, EvalMode mode = modeRtCt);


    /// The concept associated with a ConceptType. Can be nullptr if no concept is used
    /// Works only for concept types.
    Node* conceptOfType(TypeRef type);
}
