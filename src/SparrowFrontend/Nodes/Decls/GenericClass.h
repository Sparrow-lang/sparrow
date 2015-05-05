#pragma once

#include "Generic.h"

FWD_CLASS1(SprFrontend, SprClass);

namespace SprFrontend
{
    /// Class that implements a class generic
    class GenericClass : public Generic
    {
        DEFINE_NODE(GenericClass, nkSparrowDeclGenericClass, "Sparrow.Decl.GenericClass");

    public:
        explicit GenericClass(SprClass* originalClass, NodeList* parameters, Node* ifClause);

        virtual size_t paramsCount() const;
        virtual Node* param(size_t idx) const;
        virtual Instantiation* canInstantiate(const NodeVector& args);
        virtual Node* instantiateGeneric(const Location& loc, CompilationContext* context, const NodeVector& args, Instantiation* instantiation);
    };
}
