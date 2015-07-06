#pragma once

#include <Nest/Intermediate/Modifier.h>

namespace SprFrontend
{
    /// Modifier that add default members to a class, if the class doesn't have them
    class IntModClassMembers : public Nest::Modifier
    {
    public:
        virtual void afterComputeType(DynNode* node);
    };
}
