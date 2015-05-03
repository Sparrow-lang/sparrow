#pragma once

#include <Feather/FeatherNodeCommonsH.h>

namespace Feather
{
    /// Intermediate code node that holds a destruct action for the whole program.
    /// Nodes of this type should be handled at the end of the program execution, after leaving the main method
    class GlobalDestructAction : public Node
    {
        DEFINE_NODE(GlobalDestructAction, nkFeatherGlobalDestructAction, "Feather.GlobalDestructAction");

    public:
        GlobalDestructAction(const Location& loc, Node* action);

        /// Getter for the destruct action held by this node
        Node* destructAction() const;

    public:
        virtual void dump(ostream& os) const;

    protected:
        virtual void doSemanticCheck();
    };
}
