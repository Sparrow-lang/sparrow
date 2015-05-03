#include <StdInc.h>
#include "Void.h"

#include <Nest/Intermediate/TypeStock.h>

using namespace Feather;

Void::Void(EvalMode mode)
    : Type(typeVoid, mode)
{
    SET_TYPE_DESCRIPTION(*this);
}

Void* Void::get(EvalMode mode)
{
    ENTER_TIMER_DESC(Nest::theCompiler().timingSystem(), "type.get", "Get type");
    ENTER_TIMER_DESC(Nest::theCompiler().timingSystem(), "type.get.void", "  Get Void type");

    return typeStock.getCreateType<Void*>(typeVoid, (int) mode,
        [](void* p, int mode) { return new (p) Void((EvalMode) mode); } );
}

string Void::toString() const
{
    switch ( mode() )
    {
    case modeCt:    return "Void/ct";
    case modeRtCt:  return "Void/rtct";
    default:        return "Void";
    }
}

Void* Void::changeMode(EvalMode newMode)
{
    return Void::get(newMode);
}
