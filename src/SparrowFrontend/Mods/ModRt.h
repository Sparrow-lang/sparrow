#pragma once

#include <Nest/Intermediate/Modifier.h>

namespace SprFrontend
{
    /// The 'rt' modifier
    class ModRt : public Nest::Modifier
    {
    public:
        virtual void beforeSetContext(DynNode* node);
    };
}
