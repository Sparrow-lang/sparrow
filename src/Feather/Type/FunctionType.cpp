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
    subTypes_.insert(subTypes_.begin(), subTypes.begin(), subTypes.end());
    SET_TYPE_DESCRIPTION(*this);
}

size_t FunctionType::noParameters() const
{
    return subTypes_.size()-1;
}

Type* FunctionType::getParameter(size_t idx) const
{
    return subTypes_[idx+1];
}

Type* FunctionType::resultType() const
{
    return subTypes_[0];
}

vector<Type*> FunctionType::paramTypes() const
{
    return vector<Type*>(subTypes_.begin()+1, subTypes_.end());
}


string FunctionType::toString() const
{
    string res = "(";

    for ( size_t i=1; i<subTypes_.size(); ++i )
    {
        if ( i>1 )
            res += ", ";
        ASSERT(subTypes_[i]);
        res += subTypes_[i]->toString();
    }
    res += "): ";
    ASSERT(subTypes_[0]);
    res += subTypes_[0]->toString();
    return res;
}

FunctionType* FunctionType::changeMode(EvalMode newMode)
{
    return FunctionType::get(subTypes_[0], paramTypes(), newMode);
}