#pragma once

#include "Generic.h"

namespace SprFrontend
{
    /// Class that implements a class generic
    class GenericClass : public Generic
    {
        DEFINE_NODE(GenericClass, nkSparrowDeclGenericClass, "Sparrow.Decl.GenericClass");

    public:
        explicit GenericClass(Node* originalClass, Node* parameters, Node* ifClause);

        size_t paramsCount() const;
        Node* param(size_t idx) const;
        Instantiation* canInstantiate(const NodeVector& args);
        Node* instantiateGeneric(const Location& loc, CompilationContext* context, const NodeVector& args, Instantiation* instantiation);
    };
}
