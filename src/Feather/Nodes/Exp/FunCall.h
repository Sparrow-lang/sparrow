#pragma once

#include <Feather/FeatherNodeCommonsH.h>

FWD_CLASS1(Feather, Function);

namespace Feather
{
    /// Intermediate code node that represents a function call operation
    /// Given a set of arguments, this will call a function decl and return as specified by the function decl
    class FunCall : public DynNode
    {
        DEFINE_NODE(FunCall, nkFeatherExpFunCall, "Feather.Exp.FunCall");

    public:
        FunCall(const Location& loc, Function* funDecl, DynNodeVector args = {});

        /// Getter for the function decl to be called
        Function* funDecl() const;

        /// Getter for the arguments of this function call
        const DynNodeVector& arguments() const;

    public:
        virtual void dump(ostream& os) const;

    protected:
        virtual void doSemanticCheck();
    };
}
