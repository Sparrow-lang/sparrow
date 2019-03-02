#pragma once

#include "Feather/Utils/cppif/FeatherNodes.hpp"

using LocationGenFun = std::function<Nest::Location()>;

//! A generator for locations -- set by the fixture
extern LocationGenFun g_LocationGen;