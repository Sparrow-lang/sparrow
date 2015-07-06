#pragma once

#include "Generic.h"

FWD_CLASS1(SprFrontend, SprFunction);
FWD_CLASS1(Feather, Class);

namespace SprFrontend
{
    /// Class that implements the generics behavior of a declaration
    class GenericFunction : public Generic
    {
        DEFINE_NODE(GenericFunction, nkSparrowDeclGenericFunction, "Sparrow.Decl.GenericFunction");

    public:
        ~GenericFunction();

        /// Checks if the given declaration with the given parameters is a generic; if yes, creates an object of this type
        static GenericFunction* createGeneric(SprFunction* originalFun, NodeList* parameters, DynNode* ifClause, Feather::Class* thisClass = nullptr);

        virtual size_t paramsCount() const;
        virtual DynNode* param(size_t idx) const;
        virtual Instantiation* canInstantiate(const DynNodeVector& args);
        virtual DynNode* instantiateGeneric(const Location& loc, CompilationContext* context, const DynNodeVector& args, Instantiation* instantiation);

    private:
        GenericFunction(SprFunction* originalFun, DynNodeVector params, DynNodeVector genericParams, DynNode* ifClause);

    private:
        /// All the parameters of the declaration (bound + unbound) - here we have the original parameters of the declaration
        const DynNodeVector& params() const;
    };
}
