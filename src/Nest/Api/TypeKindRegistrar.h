#pragma once

#include "TypeRef.h"
#include "EvalMode.h"

#ifdef __cplusplus
extern "C" {
#endif

/// Type of function that changes the evaluation mode of a type
typedef TypeRef (*FChangeTypeMode)(TypeRef, EvalMode);

/// Registers a new type kind
///
/// @param funChangeTypeMode    The function used to change the mode of a type
/// @return the ID of the new type kind
int Nest_registerTypeKind(FChangeTypeMode funChangeTypeMode);

/// Getter for the function that can change the type mode for the given type kind
FChangeTypeMode Nest_getChangeTypeModeFun(int typeKind);

#ifdef __cplusplus
}
#endif

