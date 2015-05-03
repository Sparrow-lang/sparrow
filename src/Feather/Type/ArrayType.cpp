#include <StdInc.h>
#include "ArrayType.h"

#include <Nest/Common/Tuple.h>
#include <Nest/Intermediate/TypeStock.h>

using namespace Feather;
using namespace Nest;

ArrayType::ArrayType(StorageType* unitType, uint32_t count)
    : StorageType(typeArray, unitType->mode())
{
    subTypes_.push_back(unitType);
    flags_ = count;
    SET_TYPE_DESCRIPTION(*this);
}

ArrayType* ArrayType::get(StorageType* unitType, uint32_t count)
{
    ENTER_TIMER_DESC(Nest::theCompiler().timingSystem(), "type.get", "Get type");
    ENTER_TIMER_DESC(Nest::theCompiler().timingSystem(), "type.get.ArrayType", "  Get ArrayType");

    typedef Tuple2<StorageType*, uint32_t> Key;

    Key key(unitType, count);

    return typeStock.getCreateType<ArrayType*>(typeArray, key,
        [](void* p, const Key& key) { return new (p) ArrayType(key._1, key._2); } );
}

StorageType* ArrayType::unitType() const
{
    return static_cast<StorageType*>(subTypes_.front());
}

uint32_t ArrayType::count() const
{
    return (uint32_t) flags_;
}

Class* ArrayType::classDecl() const
{
    return unitType()->classDecl();
}

uint8_t ArrayType::noReferences() const
{
    return 0;
}

string ArrayType::toString() const
{
    ostringstream oss;
    oss << unitType()->toString() << "[" << count() << "]";
    return oss.str();
}

bool ArrayType::canBeUsedAtRt() const
{
    return unitType()->canBeUsedAtRt();
}

bool ArrayType::canBeUsedAtCt() const
{
    return unitType()->canBeUsedAtCt();
}

ArrayType* ArrayType::changeMode(EvalMode newMode)
{
    Type* newUnitType = unitType()->changeMode(newMode);
    return newUnitType ? ArrayType::get((StorageType*) newUnitType, count()) : nullptr;
}
