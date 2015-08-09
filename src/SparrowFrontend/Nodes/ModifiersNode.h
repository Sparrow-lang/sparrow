#pragma once

#include <NodeCommonsH.h>

#include <vector>

namespace SprFrontend
{
    /// Node that holds a list of modifiers to be used for another node
    class ModifiersNode : public DynNode
    {
        DEFINE_NODE(ModifiersNode, nkSparrowModifiersNode, "Sparrow.ModifiersNode");

    public:
        ModifiersNode(const Location& loc, Node* base, Node* modifierNodes);

        Node* base() const { return data_.children[0]; }

    protected:
        void doSetContextForChildren();
        void doComputeType();
        void doSemanticCheck();

    private:
        void interpretModifiers();
        void applyModifier(Node* modNode);
    };
}
