#pragma once

#include <Nest/Intermediate/Modifier.h>

namespace SprFrontend
{
    /// The initCtor modifier
    class ModMacro : public Nest::Modifier
    {
    public:
        virtual void beforeComputeType(Node* node);
    };
}
