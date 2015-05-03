#pragma once

#include <NodeCommonsH.h>

namespace SprFrontend
{
    /// A for declaration
    class For : public Node
    {
        DEFINE_NODE(For, nkSparrowStmtFor, "Sparrow.Stmt.For");

    public:
        For(const Location& loc, string name, Node* range, Node* action, Node* typeExpr = nullptr, bool ct = false);

    protected:
        virtual void doSetContextForChildren();
        virtual void doComputeType();
        virtual void doSemanticCheck();
    };
}
