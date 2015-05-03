#pragma once

#include <NodeCommonsH.h>

#include <vector>

namespace SprFrontend
{
    /// Node that holds a list of modifiers to be used for another node
    class ModifiersNode : public Node
    {
        DEFINE_NODE(ModifiersNode, nkSparrowModifiersNode, "Sparrow.ModifiersNode");

    public:
        ModifiersNode(const Location& loc, Node* base, Node* modifierNodes);

        Node* base() const { return children_[0]; }

    protected:
        virtual void doSetContextForChildren();
        virtual void doComputeType();
        virtual void doSemanticCheck();

    private:
        void interpretModifiers();
        void applyModifier(Node* modNode);
    };
}
