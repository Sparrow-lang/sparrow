#pragma once

#include <Feather/FeatherNodeCommonsH.h>

namespace Feather
{
    /// Intermediate code node that represents a standard while statement
    /// The return type will be Void
    class While : public DynNode
    {
        DEFINE_NODE(While, nkFeatherStmtWhile, "Feather.Stmt.While");

    public:
        While(const Location& location, DynNode* condition, DynNode* body, DynNode* step = nullptr, bool isCt = false);

        /// Getter for the condition node of the while statement
        DynNode* condition() const;

        /// Getter for the body of the while statement
        DynNode* body() const;

        /// Getter for the step clause of the while statement
        DynNode* step() const;

    public:
        void dump(ostream& os) const;

    protected:
        void doSetContextForChildren();
        void doSemanticCheck();
    };
}
