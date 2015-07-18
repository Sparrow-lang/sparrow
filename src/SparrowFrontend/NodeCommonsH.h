#pragma once

#include <Nest/Intermediate/SymTab.h>
#include <Nest/Intermediate/Type.h>

#include <Feather/Nodes/DynNode.h>
#include <Feather/Nodes/NodeList.h>

#include "Nodes/SparrowNodes.h"

FWD_CLASS1(Feather, NodeList)

namespace SprFrontend
{
    using Nest::Location;
    using Nest::CompilationContext;
    using Feather::DynNode;
    using Feather::DynNodeVector;
    using Nest::TypeRef;
    using Nest::SymTab;

    using Feather::NodeList;

    using Nest::Modifier;
}
