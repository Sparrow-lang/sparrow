#pragma once

#include "Nest/Utils/cppif/Type.hpp"
#include "Nest/Utils/cppif/NodeHandle.hpp"

namespace Nest {

/**
 * @brief   Base class for all wrapper structs of types with storage.
 *
 * You cannot directly create objects of this type. Used derived classes
 *
 * @see     Type
 */
struct TypeWithStorage : Type {
    TypeWithStorage() = default;
    TypeWithStorage(Nest::TypeRef type);

    //!@{ Type properties

    //! Returns the number of references used for this type
    int numReferences() const { return type_->numReferences; }

    //! Returns the referred node; the node that introduced this type
    Nest::NodeHandle referredNode() const { return type_->referredNode; }

    //!@}
    //!@{ Operations

    //! @copydoc Type::changeMode
    TypeWithStorage changeMode(Nest::EvalMode mode, Nest::Location loc = Nest::Location{}) {
        return TypeWithStorage(Type::changeMode(mode, loc));
    }

    //!@}
};

} // namespace Nest
