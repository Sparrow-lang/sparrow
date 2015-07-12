#pragma once

#include <Nest/Intermediate/Modifier.h>

namespace SprFrontend
{
    /// The native modifier
    class ModNative : public Nest::Modifier
    {
    public:
        explicit ModNative(const string& name);
        virtual void beforeComputeType(Node* node);

    private:
        string name_;
    };
}
