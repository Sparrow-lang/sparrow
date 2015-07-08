#pragma once

#include <NodeCommonsH.h>

namespace SprFrontend
{
    /// A for declaration
    class For : public DynNode
    {
        DEFINE_NODE(For, nkSparrowStmtFor, "Sparrow.Stmt.For");

    public:
        For(const Location& loc, string name, DynNode* range, DynNode* action, DynNode* typeExpr = nullptr, bool ct = false);

    protected:
        void doSetContextForChildren();
        void doComputeType();
        void doSemanticCheck();
    };
}
