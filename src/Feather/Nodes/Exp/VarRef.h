#pragma once

#include <Feather/FeatherNodeCommonsH.h>

namespace Feather
{
    /// Intermediate code node that represents a reference to a variable, a use of the variable
    /// Given the variable, this node will have the same type as the variable
    class VarRef : public DynNode
    {
        DEFINE_NODE(VarRef, nkFeatherExpVarRef, "Feather.Exp.VarRef");

    public:
        VarRef(const Location& loc, DynNode* var);

        /// Getter for the variable this node refers to
        DynNode* variable() const;

    public:
        virtual void dump(ostream& os) const;

    protected:
        virtual void doSemanticCheck();
    };
}
