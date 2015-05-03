#pragma once

#include <Nest/Intermediate/Node.h>

namespace SprFrontend
{
    /// Class that represents a lambda function expression
    class LambdaFunction : public Nest::Node
    {
        DEFINE_NODE(LambdaFunction, nkSparrowExpLambdaFunction, "Sparrow.Exp.LambdaFunction");

    public:
        LambdaFunction(const Location& loc, NodeList* parameters, Node* returnType, Node* body, NodeList* closureParams);

        virtual void dump(ostream& os) const;

    protected:
        virtual void doSemanticCheck();
    };
}
