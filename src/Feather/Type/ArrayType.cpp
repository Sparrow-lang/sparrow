#include <StdInc.h>
#include "ArrayType.h"

#include <Feather/FeatherTypes.h>
#include <Feather/Nodes/Decls/Class.h>

#include <Nest/Common/Tuple.h>
#include <Nest/Intermediate/TypeStock.h>

using namespace Feather;
using namespace Nest;

ArrayType* ArrayType::get(StorageType* unitType, uint32_t count)
{
    ENTER_TIMER_DESC(Nest::theCompiler().timingSystem(), "type.get", "Get type");
    ENTER_TIMER_DESC(Nest::theCompiler().timingSystem(), "type.get.ArrayType", "  Get ArrayType");

    TypeData* baseType = getArrayType(unitType->data_, count);

    return typeStock.getCreateType<ArrayType>(baseType);
}

StorageType* ArrayType::unitType() const
{
    return static_cast<StorageType*>(Type::fromBasicType(data_->subTypes[0]));
}

uint32_t ArrayType::count() const
{
    return (uint32_t) data_->flags;
}
