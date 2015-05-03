#pragma once

#include "AccessType.h"
#include <NodeCommonsH.h>

namespace SprFrontend
{
    /// A Sparrow class declaration
    class SprClass : public Node
    {
        DEFINE_NODE(SprClass, nkSparrowDeclSprClass, "Sparrow.Decl.Class");

    public:
        SprClass(const Location& loc, string name, NodeList* parameters,  NodeList* baseClasses, NodeList* children, Node* ifClause, AccessType accessType = publicAccess);

        NodeList* baseClasses() const;
        NodeList* classChildren() const;

        void addChild(Node* child);

        virtual void dump(ostream& os) const;

    protected:
        virtual void doSetContextForChildren();
        virtual void doComputeType();
        virtual void doSemanticCheck();
    };
}
