#pragma once

#include "AccessType.h"
#include <NodeCommonsH.h>

namespace SprFrontend
{
    /// Using declaration
    class Using : public DynNode
    {
        DEFINE_NODE(Using, nkSparrowDeclUsing, "Sparrow.Decl.Using");

    public:
        Using(const Location& loc, string alias, DynNode* usingNode, AccessType accessType = publicAccess);

        DynNode* source() const;

    protected:
        virtual void doSetContextForChildren();
        virtual void doComputeType();
        virtual void doSemanticCheck();
    };
}
