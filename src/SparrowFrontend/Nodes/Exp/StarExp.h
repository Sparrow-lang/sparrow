#pragma once

#include <NodeCommonsH.h>

namespace SprFrontend
{
    /// An star expression - * terminator of a qualified ID denoting all sparrow declaration
    class StarExp : public DynNode
    {
        DEFINE_NODE(StarExp, nkSparrowExpStarExp, "Sparrow.Exp.StarExp");

    public:
        StarExp(const Location& loc, DynNode* base);

    protected:
        virtual void doSemanticCheck();
    };
}
