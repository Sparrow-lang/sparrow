#pragma once

#include <NodeCommonsH.h>

namespace SprFrontend
{
    /// A return statement that can apply conversions
    class SprReturn : public Node
    {
        DEFINE_NODE(SprReturn, nkSparrowStmtSprReturn, "Sparrow.Stmt.SprReturn");

    public:
        SprReturn(const Location& loc, Node* exp);

    protected:
        virtual void doSemanticCheck();
    };
}
