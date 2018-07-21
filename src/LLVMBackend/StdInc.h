#pragma once

#include "Nest/Utils/DefaultStdInc.hpp"
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

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Function.h>
