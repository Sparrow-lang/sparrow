#pragma once

#include "Callable.h"

namespace SprFrontend
{
    /// A concept-check expression, as a callable object
    class ConceptCallable : public Callable
    {
    public:
        ConceptCallable(Node* concept);

        virtual const Location& location() const;
        virtual string toString() const;
        virtual size_t paramsCount() const;
        virtual Node* param(size_t idx) const;
        virtual TypeRef paramType(size_t idx) const;
        virtual EvalMode evalMode() const;
        virtual bool isAutoCt() const;

        virtual Node* generateCall(const Location& loc);

    private:
        Node* concept_;
    };
}
