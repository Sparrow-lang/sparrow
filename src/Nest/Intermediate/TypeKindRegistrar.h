#pragma once

#include "TypeRef.h"
#include "EvalMode.h"

namespace Nest
{
    /// Type of function that changes the evaluation mode of a type
    typedef TypeRef (*FChangeTypeMode)(TypeRef, EvalMode);

    /// Registers a new type kind
    ///
    /// @param funChangeTypeMode    The function used to change the mode of a type
    /// @return the ID of the new type kind
    int registerTypeKind(FChangeTypeMode funChangeTypeMode);

    /// Getter for the function that can change the type mode for the given type kind
    FChangeTypeMode getChangeTypeModeFun(int typeKind);
}
