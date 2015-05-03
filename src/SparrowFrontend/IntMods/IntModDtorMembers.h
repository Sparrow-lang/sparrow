#pragma once

#include <Nest/Intermediate/Modifier.h>

namespace SprFrontend
{
    /// Modifier that add default dtor calls for all the members
    class IntModDtorMembers : public Nest::Modifier
    {
    public:
        virtual void beforeSemanticCheck(Node* node);
    };
}
