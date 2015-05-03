#pragma once

#include <Feather/FeatherNodeCommonsH.h>

FWD_CLASS1(Feather, Function);

namespace Feather
{
    /// Intermediate code node that represents return statement
    /// The return type will be Void
    class Return : public Node
    {
        DEFINE_NODE(Return, nkFeatherStmtReturn, "Feather.Stmt.Return");

    public:
        explicit Return(const Location& location, Node* exp = nullptr);

        /// The expression given to the return node, if this returns an expression
        Node* expression() const;

        /// Getter for the function declaration that contains this node; set during compilation
        Function* parentFun() const;

    public:
        virtual void dump(ostream& os) const;

    protected:
        virtual void doSemanticCheck();
    };
}
