#pragma once

#include <NodeCommonsH.h>

namespace SprFrontend
{
    /// A for declaration
    class For : public DynNode
    {
        DEFINE_NODE(For, nkSparrowStmtFor, "Sparrow.Stmt.For");

    public:
        For(const Location& loc, string name, Node* range, Node* action, Node* typeExpr = nullptr, bool ct = false);

    protected:
        void doSetContextForChildren();
        void doComputeType();
        void doSemanticCheck();
    };
}
