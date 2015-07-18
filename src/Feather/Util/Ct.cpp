#include <StdInc.h>
#include "Ct.h"
#include <Nodes/Exp/CtValue.h>
#include <Nest/Common/Diagnostic.h>

using namespace Feather;

bool Feather::getBoolCtValue(Nest::Node* ctVal)
{
    if ( ctVal->nodeKind != nkFeatherExpCtValue )
        REP_INTERNAL(ctVal->location, "Invalid CtValue");
    CtValue* ctValue = (CtValue*) ctVal;
    return (0 != *ctValue->value<unsigned char>());
}
