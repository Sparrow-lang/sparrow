#pragma once

#include <NodeCommonsH.h>

namespace SprFrontend
{
    /// A function application (function call, or generic instantiation)
    class FunApplication : public DynNode
    {
        DEFINE_NODE(FunApplication, nkSparrowExpFunApplication, "Sparrow.Exp.FunApplication");

    public:
        FunApplication(const Location& loc, Node* base, Node* arguments);
        FunApplication(const Location& loc, Node* base, NodeVector args);

        Node* base() const;
        Node* arguments() const;

    protected:
        void doSemanticCheck();

    private:
        void checkStaticCast();
        void checkReinterpretCast();
        void checkSizeOf();
        void checkTypeOf();
        void checkIsValid();
        void checkIsValidAndTrue();
        void checkValueIfValid();
        void checkCtEval();
        void checkLift();
    };
}
