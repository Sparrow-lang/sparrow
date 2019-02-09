#pragma once

#include "Nest/Utils/cppif/Fwd.hpp"

namespace Nest {

struct CCLoc {
    CompilationContext* context_;
    Location loc_;
};

} // namespace Nest