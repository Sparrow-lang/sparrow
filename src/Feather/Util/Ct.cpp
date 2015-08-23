#include <StdInc.h>
#include "Ct.h"
#include <Feather/Nodes/FeatherNodes.h>
#include <Nest/Common/Diagnostic.h>
#include <Nest/Intermediate/Node.h>

using namespace Feather;

TypeRef Feather::getCtValueType(Nest::Node* ctVal)
{
    if ( ctVal->nodeKind != nkFeatherExpCtValue )
        REP_INTERNAL(ctVal->location, "Invalid CtValue");
    return getCheckPropertyType(ctVal, "valueType");
}

/// Getter for the value data of a CtValue node -- the data is encoded in a string
const string& Feather::getCtValueDataAsString(Nest::Node* ctVal)
{
    if ( ctVal->nodeKind != nkFeatherExpCtValue )
        REP_INTERNAL(ctVal->location, "Invalid CtValue");
    return getCheckPropertyString(ctVal, "valueData");
}

bool Feather::getBoolCtValue(Nest::Node* ctVal)
{
    return (0 != *getCtValueData<unsigned char>(ctVal));
}

bool Feather::sameCtValue(Nest::Node* ctVal1, Nest::Node* ctVal2)
{
    return getCtValueType(ctVal1) == getCtValueType(ctVal2)
        && getCtValueDataAsString(ctVal1) == getCtValueDataAsString(ctVal2);
}
