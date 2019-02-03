#pragma once

#include "Nest/Utils/cppif/TypeWithStorage.hpp"
#include "Nest/Utils/cppif/NodeHandle.hpp"

struct SampleTypes;


//! Generator for arbitrary values for the given type.
//! The type of the value matches perfectly the given type
rc::Gen<Nest::NodeHandle> arbValueForType(
        Nest::TypeWithStorage t, const SampleTypes* sampleTypes = nullptr);

//! Generator for arbitrary values for the given type (ignoring mode).
//! Ignoring the mode, the type of the generated values matches the given type
rc::Gen<Nest::NodeHandle> arbValueForTypeIgnoreMode(Nest::TypeWithStorage t);

//! Generator of values that is convertible to the given type.
rc::Gen<Nest::NodeHandle> arbValueConvertibleTo(
        Nest::TypeWithStorage t, const SampleTypes* sampleTypes = nullptr);

//! Given a CT type or a Concept type, this return a generator of bound values for the type
//! The returned values will always be CT. For concepts we generate a type node.
rc::Gen<Nest::NodeHandle> arbBoundValueForType(Nest::TypeWithStorage t, const SampleTypes& sampleTypes);
