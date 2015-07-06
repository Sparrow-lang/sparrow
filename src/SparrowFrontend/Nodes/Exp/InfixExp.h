#pragma once

#include <NodeCommonsH.h>

namespace SprFrontend
{
    /// An infix operator call
    /// When this node is constructed, the actual order of evaluation isn't decided yet. This node will actually decide
    /// the order of evaluation of the operands
    class InfixExp : public DynNode
    {
        DEFINE_NODE(InfixExp, nkSparrowExpInfixExp, "Sparrow.Exp.InfixExp");

    public:
        InfixExp(const Location& loc, string op, DynNode* arg1, DynNode* arg2);

        const string& operation() const;
        DynNode* arg1() const;
        DynNode* arg2() const;

        virtual void dump(ostream& os) const;

    protected:
        virtual void doSemanticCheck();

    private:
        void handlePrecedence();
        void handleAssociativity();

        int getPrecedence();
        bool isRightAssociativity();
        int getIntValue(const DynNodeVector& decls, int defaultVal);

        /// Swap this with the left argument
        void swapLeft();
        void swapRight();
    };
}
