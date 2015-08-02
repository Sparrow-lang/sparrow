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
        explicit GenericClass(SprClass* originalClass, Node* parameters, DynNode* ifClause);

        size_t paramsCount() const;
        DynNode* param(size_t idx) const;
        Instantiation* canInstantiate(const DynNodeVector& args);
        DynNode* instantiateGeneric(const Location& loc, CompilationContext* context, const DynNodeVector& args, Instantiation* instantiation);
    };
}
