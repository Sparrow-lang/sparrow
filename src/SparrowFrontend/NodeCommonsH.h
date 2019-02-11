#pragma once

#include "Nest/Api/SymTab.h"
#include "Nest/Api/Type.h"
#include "Nest/Api/Node.h"

#include "Nodes/SparrowNodes.h"
#include "Nest/Utils/cppif/NodeHandle.hpp"
#include "Nest/Utils/cppif/NodeRange.hpp"
#include "Nest/Utils/cppif/TypeWithStorage.hpp"
#include "Nest/Utils/cppif/CCLoc.hpp"

namespace SprFrontend {

using Nest::CCLoc;
using Nest::CompilationContext;
using Nest::Location;
using Nest::Node;
using Nest::NodeHandle;
using Nest::NodeRange;
using Nest::Range;
using Nest::StringRef;
using Nest::Type;
using Nest::TypeWithStorage;

} // namespace SprFrontend