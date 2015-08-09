#pragma once

#include <NodeCommonsH.h>

namespace SprFrontend
{
    /// A return statement that can apply conversions
    class SprReturn : public DynNode
    {
        DEFINE_NODE(SprReturn, nkSparrowStmtSprReturn, "Sparrow.Stmt.SprReturn");

    public:
        SprReturn(const Location& loc, Node* exp);

    protected:
        void doSemanticCheck();
    };
}
