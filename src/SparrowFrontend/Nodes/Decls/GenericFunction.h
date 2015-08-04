#pragma once

#include "Generic.h"

FWD_CLASS1(SprFrontend, SprFunction);

namespace SprFrontend
{
    /// Class that implements the generics behavior of a declaration
    class GenericFunction : public Generic
    {
        DEFINE_NODE(GenericFunction, nkSparrowDeclGenericFunction, "Sparrow.Decl.GenericFunction");

    public:
        ~GenericFunction();

        /// Checks if the given declaration with the given parameters is a generic; if yes, creates an object of this type
        static GenericFunction* createGeneric(SprFunction* originalFun, Node* parameters, DynNode* ifClause, Node* thisClass = nullptr);

        size_t paramsCount() const;
        DynNode* param(size_t idx) const;
        Instantiation* canInstantiate(const DynNodeVector& args);
        DynNode* instantiateGeneric(const Location& loc, CompilationContext* context, const DynNodeVector& args, Instantiation* instantiation);

    private:
        GenericFunction(SprFunction* originalFun, DynNodeVector params, DynNodeVector genericParams, DynNode* ifClause);

    private:
        /// All the parameters of the declaration (bound + unbound) - here we have the original parameters of the declaration
        const DynNodeVector& params() const;
    };
}
