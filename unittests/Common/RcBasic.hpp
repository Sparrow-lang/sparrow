#pragma once

#include "Nest/Api/EvalMode.h"

namespace rc {

//! Arbitrary definition for EvalMode; can generate RT and CT modes values
template <> struct Arbitrary<EvalMode> {
    static Gen<EvalMode> arbitrary() { return gen::element(modeRt, modeCt); }
};

} // namespace rc
