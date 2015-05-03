#pragma once

#include <Feather/FeatherNodeCommonsH.h>

namespace Feather
{
    /// Intermediate code node that represents a local space.
    /// This node will contain zero, one or more children.
    /// The type of this node is Void
    class LocalSpace : public Node
    {
        DEFINE_NODE(LocalSpace, nkFeatherLocalSpace, "Feather.LocalSpace");

    public:
        LocalSpace(const Nest::Location& location, NodeVector children = {});

        /// Adds a child node to this node
        void addChild(Node* p);
        void insertChildInFront(Node* p);

    public:
        virtual void dump(ostream& os) const;

    protected:
        virtual void doSetContextForChildren();
        virtual void doComputeType();
        virtual void doSemanticCheck();
    };
}
