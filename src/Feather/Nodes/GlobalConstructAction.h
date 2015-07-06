#pragma once

#include <Feather/FeatherNodeCommonsH.h>

namespace Feather
{
    /// Intermediate code node that holds a construct action for the whole program.
    /// Nodes of this type should be handled at the begining of the program execution, before entering the main method
    class GlobalConstructAction : public DynNode
    {
        DEFINE_NODE(GlobalConstructAction, nkFeatherGlobalConstructAction, "Feather.GlobalConstructAction");

    public:
        GlobalConstructAction(const Location& loc, DynNode* action);

        /// Getter for the construct action held by this node
        DynNode* constructAction() const;

    public:
        virtual void dump(ostream& os) const;

    protected:
        virtual void doSemanticCheck();
    };
}
