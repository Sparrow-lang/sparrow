#pragma once

#include <NodeCommonsH.h>

namespace SprFrontend
{
    /// An infix operator call
    /// When this node is constructed, the actual order of evaluation isn't decided yet. This node will actually decide
    /// the order of evaluation of the operands
    class InfixExp : public Node
    {
        DEFINE_NODE(InfixExp, nkSparrowExpInfixExp, "Sparrow.Exp.InfixExp");

    public:
        InfixExp(const Location& loc, string op, Node* arg1, Node* arg2);

        const string& operation() const;
        Node* arg1() const;
        Node* arg2() const;

        virtual void dump(ostream& os) const;

    protected:
        virtual void doSemanticCheck();

    private:
        void handlePrecedence();
        void handleAssociativity();

        int getPrecedence();
        bool isRightAssociativity();
        int getIntValue(const NodeVector& decls, int defaultVal);

        /// Swap this with the left argument
        void swapLeft();
        void swapRight();
    };
}
