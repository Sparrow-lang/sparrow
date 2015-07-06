#pragma once

#include <Nest/Intermediate/Modifier.h>

namespace SprFrontend
{
    /// The static modifier
    class ModStatic : public Nest::Modifier
    {
    public:
        virtual void beforeComputeType(DynNode* node);
    };
}
