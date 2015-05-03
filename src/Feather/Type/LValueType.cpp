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
    subTypes_.push_back(base);
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
    return static_cast<StorageType*>(subTypes_[0]);
}

Class* LValueType::classDecl() const
{
    return static_cast<StorageType*>(subTypes_[0])->classDecl();
}

uint8_t LValueType::noReferences() const
{
    return 1+subTypes_[0]->noReferences();
}

string LValueType::toString() const
{
    return "lv " + subTypes_[0]->toString();
}

bool LValueType::canBeUsedAtRt() const
{
    return subTypes_[0]->canBeUsedAtRt();
}

bool LValueType::canBeUsedAtCt() const
{
    return subTypes_[0]->canBeUsedAtCt();
}

LValueType* LValueType::changeMode(EvalMode newMode)
{
    return LValueType::get(subTypes_[0]->changeMode(newMode));
}
