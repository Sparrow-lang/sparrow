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
        SprConditional(const Location& location, DynNode* condition, DynNode* alt1, DynNode* alt2);

    protected:
        virtual void doSemanticCheck();
    };
}
