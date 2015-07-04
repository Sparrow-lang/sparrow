#include <StdInc.h>
#include "LValueType.h"
#include <Nodes/Decls/Class.h>

#include <Nest/Common/Diagnostic.h>
#include <Nest/Intermediate/TypeStock.h>

using namespace Feather;
using namespace Nest;

LValueType::LValueType(Type* base)
    : StorageType(typeLValue, base->mode())
{
    data_.subTypes = new Type*[1];
    data_.subTypes[0] = base;
    data_.hasStorage = 1;
    data_.numReferences = 1 + base->data_.numReferences;
    data_.canBeUsedAtCt = base->canBeUsedAtCt();
    data_.canBeUsedAtRt = base->canBeUsedAtRt();
    data_.referredNode = base->data_.referredNode;
    SET_TYPE_DESCRIPTION(*this);
}

LValueType* LValueType::get(Type* base)
{
    ENTER_TIMER_DESC(Nest::theCompiler().timingSystem(), "type.get", "Get type");
    ENTER_TIMER_DESC(Nest::theCompiler().timingSystem(), "type.get.lvalueType", "  Get LValueType");

    if ( !base || !base->hasStorage() )
        REP_INTERNAL(Nest::Location(), "Invalid type given when creating l-value type");

    return typeStock.getCreateType<LValueType*>(typeLValue, base,
        [](void* p, Type* base) { return new (p) LValueType(base); } );
}

StorageType* LValueType::baseType() const
{
    return static_cast<StorageType*>(data_.subTypes[0]);
}

string LValueType::toString() const
{
    return "lv " + data_.subTypes[0]->toString();
}

LValueType* LValueType::changeMode(EvalMode newMode)
{
    return LValueType::get(data_.subTypes[0]->changeMode(newMode));
}
