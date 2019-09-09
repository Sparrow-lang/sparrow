#pragma once

#include <NodeCommonsH.h>
#include "Nest/Api/EvalMode.h"

#include <vector>

namespace SprFrontend {
/// Given two compile-time values, checks if they are equal (by using the '==' operator)
bool ctValsEqual(Node* v1, Node* v2);

StringRef getStringCtValue(Node* val);
bool getBoolCtValue(Node* val);
int getIntCtValue(Node* val);
char* getByteRefCtValue(Node* val);
} // namespace SprFrontend
