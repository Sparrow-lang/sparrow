#pragma once

#include "Callable.h"

namespace SprFrontend
{
    /// A callable object for constructing temporary objects of a class
    class ClassCtorCallable : public Callable
    {
    public:
        ClassCtorCallable(Node* cls, Callable* baseCallable, EvalMode evalMode);

        /// Get the constructors of the given class as callable objects
        static Callables getCtorCallables(Node* cls, EvalMode evalMode);

        virtual const Location& location() const;
        virtual string toString() const;
        virtual size_t paramsCount() const;
        virtual DynNode* param(size_t idx) const;
        virtual EvalMode evalMode() const;
        virtual bool isAutoCt() const;

        virtual ConversionType canCall(CompilationContext* context, const Location& loc, const vector<TypeRef>& argTypes, EvalMode evalMode, bool noCustomCvt = false);
        virtual ConversionType canCall(CompilationContext* context, const Location& loc, const DynNodeVector& args, EvalMode evalMode, bool noCustomCvt = false);
        virtual DynNode* generateCall(const Location& loc);

    private:
        Node* cls_;
        Callable* baseCallable_;    // Function or generic
        EvalMode evalMode_;
        DynNode* tmpVar_;
        DynNode* thisArg_;
    };
}
