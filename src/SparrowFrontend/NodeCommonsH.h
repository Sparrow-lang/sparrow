#pragma once

#include <Nest/Intermediate/Node.h>
#include <Nest/Intermediate/SymTab.h>
#include <Nest/Intermediate/Type.h>

#include <Feather/Nodes/NodeList.h>

#include "Nodes/SparrowNodeKinds.h"

FWD_CLASS1(Feather, NodeList)

namespace SprFrontend
{
    using Nest::Location;
    using Nest::CompilationContext;
    using Nest::Node;
    using Nest::NodeVector;
    using Nest::TypeRef;
    using Nest::SymTab;

    using Feather::NodeList;

    using Nest::Modifier;
}
