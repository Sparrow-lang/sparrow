#include <StdInc.h>
#include "Void.h"
#include <Feather/FeatherTypes.h>

#include <Nest/Intermediate/TypeStock.h>

using namespace Feather;

Void* Void::get(EvalMode mode)
{
    ENTER_TIMER_DESC(Nest::theCompiler().timingSystem(), "type.get", "Get type");
    ENTER_TIMER_DESC(Nest::theCompiler().timingSystem(), "type.get.void", "  Get Void type");

    TypeData* baseType = getVoidType(mode);

    return typeStock.getCreateType<Void>(baseType);
}
