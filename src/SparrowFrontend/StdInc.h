#pragma once

#include "Nest/Utils/DefaultStdInc.hpp"

#include "Nest/Api/Compiler.h"
#include "Nest/Api/Location.h"
#include "Nest/Api/Type.h"
#include "Nest/Utils/cppif/Fwd.hpp"
#include "Nest/Utils/cppif/NodeUtils.hpp"

// Common functions, exposed

// TODO: remove these
using Nest::all;
using Nest::allM;
using Nest::at;
using Nest::begin;
using Nest::end;
using Nest::fromIniList;
using Nest::size;

// To speed up the compilation of the nodes
#include "NodeCommonsCpp.h"