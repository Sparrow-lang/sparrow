#pragma once

#include "Callable.h"

#include <Nodes/Decls/Generic.h>

namespace SprFrontend
{
    using SprFrontend::Generic;

    /// A generic callable object
    class GenericCallable : public Callable
    {
    public:
        GenericCallable(Generic* generic);

        virtual const Location& location() const;
        virtual string toString() const;
        virtual size_t paramsCount() const;
        virtual Node* param(size_t idx) const;
        virtual EvalMode evalMode() const;
        virtual bool isAutoCt() const;

        virtual ConversionType canCall(CompilationContext* context, const Location& loc, const NodeVector& args, EvalMode evalMode, bool noCustomCvt = false);
        virtual Node* generateCall(const Location& loc);

    private:
        Generic* generic_;
        Instantiation* inst_;
        
        NodeVector argsWithCvt_;
    };
}
