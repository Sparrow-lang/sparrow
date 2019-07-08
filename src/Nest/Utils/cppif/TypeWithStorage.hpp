#pragma once

#include "Nest/Utils/cppif/Type.hpp"
#include "Nest/Utils/cppif/NodeHandle.hpp"

namespace Nest {

/**
 * @brief   Base class for all wrapper structs of types with storage.
 *
 * One cannot directly create objects of this type. Used derived classes.
 *
 * This doesn't expose any new functionality compared to base class Type. However, this is useful to
 * propagate the information that a type has storage (type safety).
 *
 * @see     Type
 */
struct TypeWithStorage : Type {
    TypeWithStorage() = default;
    TypeWithStorage(Nest::TypeRef type);

    //!@{ Operations

    //! @copydoc Type::changeMode
    TypeWithStorage changeMode(Nest::EvalMode mode, Nest::Location loc = Nest::Location{}) const {
        return {Type::changeMode(mode, loc)};
    }

    //!@}
};

} // namespace Nest
