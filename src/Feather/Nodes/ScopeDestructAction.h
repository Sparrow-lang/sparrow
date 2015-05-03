#pragma once

#include <Feather/FeatherNodeCommonsH.h>

namespace Feather
{
    /// Intermediate code node that holds a destruct action for the current scope.
    /// Nodes of this type should be handled at the end of scope to destruct all the scope constructs.
    class ScopeDestructAction : public Node
    {
        DEFINE_NODE(ScopeDestructAction, nkFeatherScopeDestructAction, "Feather.ScopeDestructAction");

    public:
        ScopeDestructAction(const Location& loc, Node* action);

        /// Getter for the destruct action held by this node
        Node* destructAction() const;

    public:
        virtual void dump(ostream& os) const;

    protected:
        virtual void doSemanticCheck();
    };
}
