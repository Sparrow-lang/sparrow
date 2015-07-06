#pragma once

#include <NodeCommonsH.h>
#include <Nest/Intermediate/EvalMode.h>

#include <vector>

namespace SprFrontend
{
    /// Given two compile-time values, checks if they are equal (by using the '==' operator)
    bool ctValsEqual(DynNode* v1, DynNode* v2);

    const char* getStringCtValue(DynNode* val);
    bool getBoolCtValue(DynNode* val);
    int getIntCtValue(DynNode* val);
    int* getIntRefCtValue(DynNode* val);
    size_t getSizeTypeCtValue(DynNode* val);
}
