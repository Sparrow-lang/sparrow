#pragma once

#include <Nest/Intermediate/DynNode.h>

namespace SprFrontend
{
    /// Class that represents a lambda function expression
    class LambdaFunction : public Nest::DynNode
    {
        DEFINE_NODE(LambdaFunction, nkSparrowExpLambdaFunction, "Sparrow.Exp.LambdaFunction");

    public:
        LambdaFunction(const Location& loc, NodeList* parameters, DynNode* returnType, DynNode* body, NodeList* closureParams);

        virtual void dump(ostream& os) const;

    protected:
        virtual void doSemanticCheck();
    };
}
