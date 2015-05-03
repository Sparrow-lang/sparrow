#pragma once

#include <Nest/Intermediate/Modifier.h>

namespace SprFrontend
{
    /// The 'rtct' modifier
    class ModRtCt : public Nest::Modifier
    {
    public:
        virtual void beforeSetContext(Node* node);
    };
}
