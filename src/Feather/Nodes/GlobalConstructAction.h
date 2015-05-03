#pragma once

#include <Feather/FeatherNodeCommonsH.h>

namespace Feather
{
    /// Intermediate code node that holds a construct action for the whole program.
    /// Nodes of this type should be handled at the begining of the program execution, before entering the main method
    class GlobalConstructAction : public Node
    {
        DEFINE_NODE(GlobalConstructAction, nkFeatherGlobalConstructAction, "Feather.GlobalConstructAction");

    public:
        GlobalConstructAction(const Location& loc, Node* action);

        /// Getter for the construct action held by this node
        Node* constructAction() const;

    public:
        virtual void dump(ostream& os) const;

    protected:
        virtual void doSemanticCheck();
    };
}
