#pragma once

#include <NodeCommonsH.h>

namespace SprFrontend
{
    /// The 'this' expression of the Sparrow language
    class This : public Node
    {
        DEFINE_NODE(This, nkSparrowExpThis, "Sparrow.Exp.This");

    public:
        explicit This(const Location& loc);

    protected:
        virtual void doSemanticCheck();
    };
}
