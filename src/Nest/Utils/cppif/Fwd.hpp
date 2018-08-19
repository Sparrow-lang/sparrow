#pragma once

#include "Nest/Api/EvalMode.h"

struct Nest_Backend;
struct Nest_CompilationContext;
struct Nest_CompilerModule;
struct Nest_Location;
struct Nest_LocationPos;
struct Nest_Modifier;
struct Nest_ModifiersArray;
struct Nest_Node;
struct Nest_NodeArray;
struct Nest_NodeProperties;
struct Nest_NodeProperty;
struct Nest_NodeRange;
struct Nest_NodeRangeM;
struct Nest_PtrRange;
struct Nest_SourceCode;
struct Nest_StringRef;
struct Nest_SymTab;
struct Nest_Type;
using Nest_TypeRef = const struct Nest_Type*;

namespace Nest {

using CompilationContext = Nest_CompilationContext;
using Location = Nest_Location;
using Node = Nest_Node;
using TypeRef = Nest_TypeRef;

using EvalMode = Nest_EvalMode;

struct StringRef;
struct NodeHandle;
struct NodeRange;
struct NodeRangeM;

} // namespace Nest
