#pragma once

#include <NodeCommonsH.h>

#include <vector>

namespace SprFrontend
{
    /// DynNode that holds a list of modifiers to be used for another node
    class ModifiersNode : public DynNode
    {
        DEFINE_NODE(ModifiersNode, nkSparrowModifiersNode, "Sparrow.ModifiersNode");

    public:
        ModifiersNode(const Location& loc, DynNode* base, DynNode* modifierNodes);

        DynNode* base() const { return (DynNode*) data_.children[0]; }

    protected:
        void doSetContextForChildren();
        void doComputeType();
        void doSemanticCheck();

    private:
        void interpretModifiers();
        void applyModifier(DynNode* modNode);
    };
}
