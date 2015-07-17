#pragma once

#include <Feather/Nodes/DynNode.h>

namespace SprFrontend
{
    /// Class that represents a lambda function expression
    class LambdaFunction : public Feather::DynNode
    {
        DEFINE_NODE(LambdaFunction, nkSparrowExpLambdaFunction, "Sparrow.Exp.LambdaFunction");

    public:
        LambdaFunction(const Location& loc, NodeList* parameters, DynNode* returnType, DynNode* body, NodeList* closureParams);

        void dump(ostream& os) const;

    protected:
        void doSemanticCheck();
    };
}
