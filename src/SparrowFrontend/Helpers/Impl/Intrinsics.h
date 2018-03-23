#pragma once

#include <NodeCommonsH.h>
#include <Nest/Utils/NodeVector.hpp>

namespace SprFrontend {

/// Checks if the given function call is a basic intrinsic
/// If yes, generates the appropriate code for it
Node* handleIntrinsic(
        Node* fun, CompilationContext* context, const Location& loc, const NodeVector& args);
} // namespace SprFrontend
