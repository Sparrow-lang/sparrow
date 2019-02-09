#pragma once

#include "Nest/Utils/cppif/Fwd.hpp"
#include "Nest/Utils/cppif/TypeWithStorage.hpp"

namespace SprFrontend {

using Nest::Type;
using Nest::TypeWithStorage;

// The type kinds for the SparrowFrontend types
extern int typeKindConcept;

/// Called to initialize the SparrowFrontend type kinds
void initSparrowFrontendTypeKinds();

/**
 * @brief   A concept type.
 *
 * Represents a generic set of types, types that models a specific concept; that is the set of types
 * that fulfill a certain predicate.
 *
 * Concept types can be introduced by concepts, or by generics. A concept type that is introduced by
 * a generic represents the set of types that can be instantiated of that generic.
 */
struct ConceptType : TypeWithStorage {
    ConceptType() = default;
    ConceptType(Nest::TypeRef type);

    /**
     * @brief      Factory method to create ConceptType types
     *
     * @param[in]  decl           The concept/generic declaration
     * @param[in]  numReferences  The number references for this type
     * @param[in]  mode           The mode
     *
     * @return     The corresponding ConceptType
     */
    static ConceptType get(
            Nest::NodeHandle decl = {}, int numReferences = 0, Nest::EvalMode mode = modeRt);

    //! Returns the referred concept/generic
    Nest::NodeHandle decl() const;

    //! @copydoc Type::changeMode
    ConceptType changeMode(Nest::EvalMode mode, Nest::Location loc = Nest::Location{}) {
        return ConceptType(Type::changeMode(mode, loc));
    }
};

/**
 * @brief      Gets the base type of the given type
 *
 * The base type of a type would be the type obtained by dereferencing the given type to maximum.
 * If the given type will have a category type, this will remove the category.
 * Examples:
 *     Int -> Int
 *     Int const -> Int
 *     @Int -> Int
 *     @Int const -> Int
 *
 * @param[in]  t     The type to get the base type for
 *
 * @return     The base type of the given type
 */
TypeWithStorage baseType(TypeWithStorage t);

} // namespace SprFrontend
