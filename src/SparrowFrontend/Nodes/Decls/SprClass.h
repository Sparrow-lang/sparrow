#pragma once

#include "AccessType.h"
#include <NodeCommonsH.h>

namespace SprFrontend
{
    /// A Sparrow class declaration
    class SprClass : public DynNode
    {
        DEFINE_NODE(SprClass, nkSparrowDeclSprClass, "Sparrow.Decl.Class");

    public:
        SprClass(const Location& loc, string name, Node* parameters,  Node* baseClasses, Node* children, DynNode* ifClause, AccessType accessType = publicAccess);

        Node* baseClasses() const;
        Node* classChildren() const;

        void addChild(DynNode* child);

        void dump(ostream& os) const;

    protected:
        void doSetContextForChildren();
        void doComputeType();
        void doSemanticCheck();
    };
}
