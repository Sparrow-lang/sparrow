#include <StdInc.h>
#include "FunctionType.h"

#include <Feather/FeatherTypes.h>
#include <Util/TypeTraits.h>

#include <Nest/Intermediate/TypeStock.h>

namespace std
{
    template <typename T>
    struct hash<vector<T> >
    {
        size_t operator()(const vector<T>& v) const
        {
            size_t res = hash<size_t>()(v.size());
            for ( const auto& val: v )
                res = res*3079 + hash<T>()(val);
            return res;
        }
    };
}

using namespace Feather;

FunctionType* FunctionType::get(Type* resultType, const vector<Type*>& paramTypes, EvalMode mode)
{
    ENTER_TIMER_DESC(Nest::theCompiler().timingSystem(), "type.get", "Get type");
    ENTER_TIMER_DESC(Nest::theCompiler().timingSystem(), "type.get.functionType", "  Get FunctionType");

    TypeRef resType = changeTypeMode(resultType, mode)->data_;
    vector<TypeRef> params(paramTypes.size(), nullptr);
    for ( size_t i=0; i<paramTypes.size(); ++i )
        params[i] = changeTypeMode(paramTypes[i], mode)->data_;

    TypeData* baseType = getFunctionType(resType, params, mode);

    return typeStock.getCreateType<FunctionType>(baseType);
}

size_t FunctionType::noParameters() const
{
    return data_->numSubtypes-1;
}

Type* FunctionType::getParameter(size_t idx) const
{
    return Type::fromBasicType(data_->subTypes[idx+1]);
}

Type* FunctionType::resultType() const
{
    return Type::fromBasicType(data_->subTypes[0]);
}

vector<Type*> FunctionType::paramTypes() const
{
    vector<Type*> res(data_->numSubtypes-1, nullptr);
    for ( size_t i=1; i<data_->numSubtypes; ++i )
        res[i-1] = Type::fromBasicType(data_->subTypes[i]);
    return res;
}
