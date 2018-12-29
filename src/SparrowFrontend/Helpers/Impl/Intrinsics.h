#pragma once

#include <NodeCommonsH.h>
#include "Nest/Utils/cppif/NodeUtils.hpp"

namespace Feather {
struct FunctionDecl;
}

namespace SprFrontend {

using Nest::NodeHandle;
using Nest::NodeRange;

/// Checks if the given function call is a basic intrinsic
/// If yes, generates the appropriate code for it
NodeHandle handleIntrinsic(Feather::FunctionDecl fun, CompilationContext* context,
        const Location& loc, NodeRange args);
} // namespace SprFrontend
