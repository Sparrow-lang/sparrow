#pragma once

#include "Callable.h"

FWD_CLASS1(SprFrontend, SprConcept);

namespace SprFrontend
{
    /// A concept-check expression, as a callable object
    class ConceptCallable : public Callable
    {
    public:
        ConceptCallable(SprConcept* concept);

        virtual const Location& location() const;
        virtual string toString() const;
        virtual size_t paramsCount() const;
        virtual DynNode* param(size_t idx) const;
        virtual TypeRef paramType(size_t idx) const;
        virtual EvalMode evalMode() const;
        virtual bool isAutoCt() const;

        virtual DynNode* generateCall(const Location& loc);

    private:
        SprConcept* concept_;
    };
}
