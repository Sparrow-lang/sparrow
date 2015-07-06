#include <StdInc.h>
#include "Ct.h"
#include <Nodes/Exp/CtValue.h>
#include <Nest/Common/Diagnostic.h>

using namespace Feather;

bool Feather::getBoolCtValue(DynNode* ctVal)
{
    CtValue* ctValue = ctVal->as<CtValue>();
    if ( !ctValue )
        REP_INTERNAL(ctVal->location(), "Invalid CtValue");
    return (0 != *ctValue->value<unsigned char>());
}
