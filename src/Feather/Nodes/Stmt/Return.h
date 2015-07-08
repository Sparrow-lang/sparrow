#pragma once

#include <Feather/FeatherNodeCommonsH.h>

FWD_CLASS1(Feather, Function);

namespace Feather
{
    /// Intermediate code node that represents return statement
    /// The return type will be Void
    class Return : public DynNode
    {
        DEFINE_NODE(Return, nkFeatherStmtReturn, "Feather.Stmt.Return");

    public:
        explicit Return(const Location& location, DynNode* exp = nullptr);

        /// The expression given to the return node, if this returns an expression
        DynNode* expression() const;

        /// Getter for the function declaration that contains this node; set during compilation
        Function* parentFun() const;

    public:
        void dump(ostream& os) const;

    protected:
        void doSemanticCheck();
    };
}
