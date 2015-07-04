#include <StdInc.h>
#include "FunctionType.h"

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

FunctionType* FunctionType::get(Type* resultType, vector<Type*> paramTypes, EvalMode mode)
{
    ENTER_TIMER_DESC(Nest::theCompiler().timingSystem(), "type.get", "Get type");
    ENTER_TIMER_DESC(Nest::theCompiler().timingSystem(), "type.get.functionType", "  Get FunctionType");

    vector<Type*> subTypes = paramTypes;
    subTypes.insert(subTypes.begin(), resultType);
    for ( Type*& t: subTypes )
    {
        t = changeTypeMode(t, mode);
    }

    return typeStock.getCreateType<FunctionType*>(typeFunction, subTypes,
        [](void* p, const vector<Type*>& subTypes) { return new (p) FunctionType(subTypes); } );
}

FunctionType::FunctionType(const vector<Type*>& subTypes)
    : Type(typeFunction, subTypes.front()->mode())
{
    data_.numSubtypes = subTypes.size();
    data_.subTypes = new Type*[data_.numSubtypes];
    copy(subTypes.begin(), subTypes.end(), data_.subTypes);
    SET_TYPE_DESCRIPTION(*this);
}

size_t FunctionType::noParameters() const
{
    return data_.numSubtypes-1;
}

Type* FunctionType::getParameter(size_t idx) const
{
    return data_.subTypes[idx+1];
}

Type* FunctionType::resultType() const
{
    return data_.subTypes[0];
}

vector<Type*> FunctionType::paramTypes() const
{
    return vector<Type*>(data_.subTypes+1, data_.subTypes + data_.numSubtypes);
}


string FunctionType::toString() const
{
    string res = "(";

    for ( size_t i=1; i<data_.numSubtypes; ++i )
    {
        if ( i>1 )
            res += ", ";
        ASSERT(data_.subTypes[i]);
        res += data_.subTypes[i]->toString();
    }
    res += "): ";
    ASSERT(data_.subTypes[0]);
    res += data_.subTypes[0]->toString();
    return res;
}

FunctionType* FunctionType::changeMode(EvalMode newMode)
{
    return FunctionType::get(data_.subTypes[0], paramTypes(), newMode);
}