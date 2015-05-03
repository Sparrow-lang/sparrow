#pragma once

#include <Nest/Intermediate/Modifier.h>

namespace SprFrontend
{
    /// The 'ct' modifier
    class ModCt : public Nest::Modifier
    {
    public:
        virtual void beforeSetContext(Node* node);
    };
}
