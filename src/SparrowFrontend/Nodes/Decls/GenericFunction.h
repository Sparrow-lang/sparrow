#pragma once

#include "Generic.h"

namespace SprFrontend
{
    /// Class that implements the generics behavior of a declaration
    class GenericFunction : public Generic
    {
        DEFINE_NODE(GenericFunction, nkSparrowDeclGenericFunction, "Sparrow.Decl.GenericFunction");

    public:
        ~GenericFunction();

        /// Checks if the given declaration with the given parameters is a generic; if yes, creates an object of this type
        static GenericFunction* createGeneric(Node* originalFun, Node* parameters, Node* ifClause, Node* thisClass = nullptr);

        size_t paramsCount() const;
        Node* param(size_t idx) const;
        Instantiation* canInstantiate(const NodeVector& args);
        Node* instantiateGeneric(const Location& loc, CompilationContext* context, const NodeVector& args, Instantiation* instantiation);

    private:
        GenericFunction(Node* originalFun, NodeVector params, NodeVector genericParams, Node* ifClause);

    private:
        /// All the parameters of the declaration (bound + unbound) - here we have the original parameters of the declaration
        const NodeVector& params() const;
    };
}
