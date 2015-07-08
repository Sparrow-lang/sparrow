#pragma once

#include <Feather/FeatherNodeCommonsH.h>

namespace Feather
{
    /// Intermediate code node that holds a destruct action for the current scope.
    /// Nodes of this type should be handled at the end of scope to destruct all the scope constructs.
    class ScopeDestructAction : public DynNode
    {
        DEFINE_NODE(ScopeDestructAction, nkFeatherScopeDestructAction, "Feather.ScopeDestructAction");

    public:
        ScopeDestructAction(const Location& loc, DynNode* action);

        /// Getter for the destruct action held by this node
        DynNode* destructAction() const;

    public:
        void dump(ostream& os) const;

    protected:
        void doSemanticCheck();
    };
}
