#pragma once

#include <NodeCommonsH.h>

namespace SprFrontend
{
    /// An operator call
    class OperatorCall : public DynNode
    {
        DEFINE_NODE(OperatorCall, nkSparrowExpOperatorCall, "Sparrow.Exp.OperatorCall");

    public:
        // Can be used to construct the following types of operator calls:
        // - infix:     A op B  => call OperatorCall(loc, A, op, B);
        // - postfix:   A op    => call OperatorCall(loc, A, op, nullptr);
        // - prefix:    op A    => call OperatorCall(loc, nullptr, op, A);
        OperatorCall(const Location& loc, DynNode* arg1, string op, DynNode* arg2);

        void dump(ostream& os) const;

    protected:
        void doSemanticCheck();

    private:
        DynNode* selectOperator(const string& operation, DynNode* arg1, DynNode* arg2);
        void handleFApp();
        void handleDotExpr();
        void handleRefEq();
        void handleRefNe();
        void handleRefAssign();
        void handleFunPtr();
    };
}
