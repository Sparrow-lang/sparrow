#include <StdInc.h>
#include "LValueType.h"
#include <Feather/FeatherTypes.h>
#include <Nodes/Decls/Class.h>

#include <Nest/Common/Diagnostic.h>
#include <Nest/Intermediate/TypeStock.h>

using namespace Feather;
using namespace Nest;

LValueType* LValueType::get(Type* base)
{
    ENTER_TIMER_DESC(Nest::theCompiler().timingSystem(), "type.get", "Get type");
    ENTER_TIMER_DESC(Nest::theCompiler().timingSystem(), "type.get.lvalueType", "  Get LValueType");

    if ( !base || !base->hasStorage() )
        REP_INTERNAL(Nest::Location(), "Invalid type given when creating l-value type");

    TypeData* baseType = getLValueType(base->data_);

    return typeStock.getCreateType<LValueType>(baseType);
}

StorageType* LValueType::baseType() const
{
    return static_cast<StorageType*>(Type::fromBasicType(data_->subTypes[0]));
}
