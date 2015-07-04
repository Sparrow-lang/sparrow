#pragma once

#include <Nest/Intermediate/TypeRef.h>

FWD_CLASS1(SprFrontend, SprConcept);

namespace SprFrontend
{
    using namespace Nest;

    /// Returns a type that represents a concept type
    /// A type that represents a generic set of types, types that model a given concept. If no concept is given, the
    /// type will model any type
    TypeRef getConceptType(SprConcept* concept = nullptr, uint8_t numReferences = 0, Nest::EvalMode mode = Nest::modeRtCt);
}
