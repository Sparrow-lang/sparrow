#pragma once

#include "Callable.h"

namespace SprFrontend
{
    /// A normal function, as a callable object
    class FunctionCallable : public Callable
    {
    public:
        FunctionCallable(Node* fun);

        virtual const Location& location() const;
        virtual string toString() const;
        virtual size_t paramsCount() const;
        virtual DynNode* param(size_t idx) const;
        virtual EvalMode evalMode() const;
        virtual bool isAutoCt() const;

        virtual DynNode* generateCall(const Location& loc);

    private:
        Node* fun_;
        bool hasResultParam_;
    };
}
