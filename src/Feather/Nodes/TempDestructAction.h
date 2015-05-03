#pragma once

#include <Feather/FeatherNodeCommonsH.h>

namespace Feather
{
    /// Intermediate code node that holds a destruct action for a temporary.
    /// Nodes of this type should be handled by statements to destruct any temporaries created in the statement.
    class TempDestructAction : public Node
    {
        DEFINE_NODE(TempDestructAction, nkFeatherTempDestructAction, "Feather.TempDestructAction");

    public:
        TempDestructAction(const Location& loc, Node* action);

        /// Getter for the destruct action held by this node
        Node* destructAction() const;

    public:
        virtual void dump(ostream& os) const;

    protected:
        virtual void doSemanticCheck();
    };
}
