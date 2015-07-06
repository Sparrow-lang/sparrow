#pragma once

#include <Feather/FeatherNodeCommonsH.h>

namespace Feather
{
    /// Intermediate code node that represents a standard if statement
    /// The return type will be Void
    class If : public DynNode
    {
        DEFINE_NODE(If, nkFeatherStmtIf, "Feather.Stmt.If");

    public:
        If(const Location& location, DynNode* condition, DynNode* thenClause, DynNode* elseClause = nullptr, bool isCt = false);

        /// Getter for the condition node of the if statement
        DynNode* condition() const;

        /// Getter for the then-clause of the if statement
        DynNode* thenClause() const;

        /// Getter for the else-clause of the if statement
        DynNode* elseClause() const;

    public:
        virtual void dump(ostream& os) const;

    protected:
        virtual void doSetContextForChildren();
        virtual void doSemanticCheck();
    };
}
