#pragma once

#include <Nest/Intermediate/Modifier.h>

namespace SprFrontend
{
    /// Modifier that add default ctor calls for all the members for which no ctor is found
    class IntModCtorMembers : public Nest::Modifier
    {
    public:
        virtual void beforeSemanticCheck(DynNode* node);
    };
}
