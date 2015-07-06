#pragma once

#include <Nest/Intermediate/Modifier.h>

namespace SprFrontend
{
    /// The ctGeneric modifier - allows definition of generics at CT
    class ModCtGeneric : public Nest::Modifier
    {
    public:
        virtual void beforeComputeType(DynNode* node);
    };
}
