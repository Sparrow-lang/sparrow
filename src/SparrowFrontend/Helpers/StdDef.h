#pragma once

#include "Nest/Utils/cppif/Type.hpp"

namespace SprFrontend {
namespace StdDef {

using Nest::Type;

extern Type typeType;
extern Type typeVoid;
extern Type typeNull;
extern Type typeBool;
extern Type typeByte;
extern Type typeInt;
extern Type typeSizeType;
extern Type typeStringRef;

extern Type typeRefType;
extern Type typeRefByte;
extern Type typeRefInt;
extern Type typeSizeTypeCt;

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
