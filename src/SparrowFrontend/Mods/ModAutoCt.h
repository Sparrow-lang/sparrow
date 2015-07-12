#pragma once

#include <Nest/Intermediate/Modifier.h>

namespace SprFrontend
{
    /// The 'modeRtCt' modifier
    class ModAutoCt : public Nest::Modifier
    {
    public:
        virtual void beforeSetContext(Node* node);
    };
}
