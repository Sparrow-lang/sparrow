#pragma once

#include "AccessType.h"
#include <NodeCommonsH.h>

namespace SprFrontend
{
    /// Using declaration
    class Using : public Node
    {
        DEFINE_NODE(Using, nkSparrowDeclUsing, "Sparrow.Decl.Using");

    public:
        Using(const Location& loc, string alias, Node* usingNode, AccessType accessType = publicAccess);

        Node* source() const;

    protected:
        virtual void doSetContextForChildren();
        virtual void doComputeType();
        virtual void doSemanticCheck();
    };
}
