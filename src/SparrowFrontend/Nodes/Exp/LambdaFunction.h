#pragma once

#include <Nodes/DynNode.h>

namespace SprFrontend
{
    /// Class that represents a lambda function expression
    class LambdaFunction : public DynNode
    {
        DEFINE_NODE(LambdaFunction, nkSparrowExpLambdaFunction, "Sparrow.Exp.LambdaFunction");

    public:
        LambdaFunction(const Location& loc, Node* parameters, DynNode* returnType, DynNode* body, Node* closureParams);

        void dump(ostream& os) const;

    protected:
        void doSemanticCheck();
    };
}
