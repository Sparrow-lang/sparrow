#pragma once

#include <NodeCommonsH.h>

namespace SprFrontend
{
    /// Sparrow conditional expression
    /// The alternative types may not be the same, but one should implicitly convert to the other
    class SprConditional : public DynNode
    {
        DEFINE_NODE(SprConditional, nkSparrowExpSprConditional, "Sparrow.Exp.SprConditional");

    public:
        SprConditional(const Location& location, Node* condition, Node* alt1, Node* alt2);

    protected:
        void doSemanticCheck();
    };
}
