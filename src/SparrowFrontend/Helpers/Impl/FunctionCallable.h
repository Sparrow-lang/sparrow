#pragma once

#include "Callable.h"

FWD_CLASS1(Feather, Function);

namespace SprFrontend
{
    using Feather::Function;

    /// A normal function, as a callable object
    class FunctionCallable : public Callable
    {
    public:
        FunctionCallable(Function* fun);

        virtual const Location& location() const;
        virtual string toString() const;
        virtual size_t paramsCount() const;
        virtual DynNode* param(size_t idx) const;
        virtual EvalMode evalMode() const;
        virtual bool isAutoCt() const;

        virtual DynNode* generateCall(const Location& loc);

    private:
        Function* fun_;
        bool hasResultParam_;
    };
}
