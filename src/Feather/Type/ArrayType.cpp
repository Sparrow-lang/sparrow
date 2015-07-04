#include <StdInc.h>
#include "ArrayType.h"

#include <Feather/Nodes/Decls/Class.h>

#include <Nest/Common/Tuple.h>
#include <Nest/Intermediate/TypeStock.h>

using namespace Feather;
using namespace Nest;

ArrayType::ArrayType(StorageType* unitType, uint32_t count)
    : StorageType(typeArray, unitType->mode())
{
    data_.numSubtypes = 1;
    data_.hasStorage = 1;
    data_.subTypes = new Type*[1];
    data_.subTypes[0] = unitType;
    data_.flags = count;
    data_.canBeUsedAtRt = unitType->canBeUsedAtRt();
    data_.canBeUsedAtCt = unitType->canBeUsedAtCt();
    data_.referredNode = unitType->classDecl();
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
    return static_cast<StorageType*>(data_.subTypes[0]);
}

uint32_t ArrayType::count() const
{
    return (uint32_t) data_.flags;
}

string ArrayType::toString() const
{
    ostringstream oss;
    oss << unitType()->toString() << "[" << count() << "]";
    return oss.str();
}

ArrayType* ArrayType::changeMode(EvalMode newMode)
{
    Type* newUnitType = unitType()->changeMode(newMode);
    return newUnitType ? ArrayType::get((StorageType*) newUnitType, count()) : nullptr;
}
