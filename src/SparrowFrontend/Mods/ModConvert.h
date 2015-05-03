#pragma once

#include <Nest/Intermediate/Modifier.h>

namespace SprFrontend
{
    /// The convert modifier
    class ModConvert : public Nest::Modifier
    {
    public:
        virtual void beforeComputeType(Node* node);
    };
}
