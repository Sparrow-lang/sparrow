#pragma once

#include <NodeCommonsH.h>

namespace SprFrontend
{
    /// An operator call
    class OperatorCall : public Node
    {
        DEFINE_NODE(OperatorCall, nkSparrowExpOperatorCall, "Sparrow.Exp.OperatorCall");

    public:
        // Can be used to construct the following types of operator calls:
        // - infix:     A op B  => call OperatorCall(loc, A, op, B);
        // - postfix:   A op    => call OperatorCall(loc, A, op, nullptr);
        // - prefix:    op A    => call OperatorCall(loc, nullptr, op, A);
        OperatorCall(const Location& loc, Node* arg1, string op, Node* arg2);

        virtual void dump(ostream& os) const;

    protected:
        virtual void doSemanticCheck();

    private:
        Node* selectOperator(const string& operation, Node* arg1, Node* arg2);
        void handleFApp();
        void handleDotExpr();
        void handleRefEq();
        void handleRefNe();
        void handleRefAssign();
        void handleFunPtr();
    };
}
