#pragma once

#include <Feather/FeatherNodeCommonsH.h>

namespace Feather
{
    /// Intermediate code node that holds zero, one or more nodes.
    /// The contained nodes are considered to be at the same node as the current node.
    /// The type of this node will be the type of the last contained node.
    class NodeList : public DynNode
    {
        DEFINE_NODE(NodeList, nkFeatherNodeList, "Feather.NodeList");

    public:
        NodeList(const Location& loc, DynNodeVector children = {}, bool resultVoid = false);

        /// Adds a child node to this node
        void addChild(DynNode* p);

    public:
        virtual void dump(ostream& os) const;

    protected:
        virtual void doComputeType();
        virtual void doSemanticCheck();
    };
}
