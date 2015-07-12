#pragma once

#include <Nest/Intermediate/Modifier.h>

namespace SprFrontend
{
    /// The noInline modifier
    class ModNoInline : public Nest::Modifier
    {
    public:
        virtual void beforeComputeType(Node* node);
    };
}
