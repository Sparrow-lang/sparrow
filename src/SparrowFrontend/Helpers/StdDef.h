#pragma once

#include "Nest/Utils/cppif/TypeWithStorage.hpp"

namespace SprFrontend {
namespace StdDef {

using Nest::Type;
using Nest::TypeWithStorage;

extern TypeWithStorage typeType;
extern Type typeVoid;
extern TypeWithStorage typeNull;
extern TypeWithStorage typeBool;
extern TypeWithStorage typeByte;
extern TypeWithStorage typeInt;
extern TypeWithStorage typeSizeType;
extern TypeWithStorage typeStringRef;

extern TypeWithStorage typeRefType;
extern TypeWithStorage typeRefByte;
extern TypeWithStorage typeRefInt;
extern TypeWithStorage typeSizeTypeCt;

extern Node* clsType;
extern Node* clsNull;
extern Node* clsBool;

extern Node* opRefEq;
extern Node* opRefNe;
} // namespace StdDef

/// Creates the Type class & corresponding type; we need it before loading anything else
void initTypeType(CompilationContext* ctx);

void checkStdClass(Node* cls);
void checkStdFunction(Node* cls);
} // namespace SprFrontend
