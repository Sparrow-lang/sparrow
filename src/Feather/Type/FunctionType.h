#pragma once

#include <Nest/Intermediate/Type.h>

FWD_CLASS1(Feather, Class);

namespace Feather
{
    using namespace Nest;

    /// A type that represents a classic function, with parameters and result type
    /// This type can be constructed from a set of parameter types and a result type
    class FunctionType : public Nest::Type
    {
    public:
        static FunctionType* get(Type* resultType, vector<Type*> paramTypes, EvalMode mode);

        size_t noParameters() const;
        Type* getParameter(size_t idx) const;
        Type* resultType() const;
        vector<Type*> paramTypes() const;

    public:
        virtual string toString() const;
        virtual FunctionType* changeMode(EvalMode newMode);

    private:
        FunctionType(const vector<Type*>& subTypes);
    };
}
