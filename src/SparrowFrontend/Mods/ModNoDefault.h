#pragma once

#include <Nest/Intermediate/Modifier.h>

namespace SprFrontend
{
    /// The noDefault modifier
    class ModNoDefault : public Nest::Modifier
    {
    public:
        virtual void beforeComputeType(Node* node);
    };
}
