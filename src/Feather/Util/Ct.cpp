#include <StdInc.h>
#include "Ct.h"
#include <Feather/Nodes/FeatherNodes.h>
#include <Nest/Common/Diagnostic.hpp>
#include <Nest/Intermediate/Node.h>

using namespace Feather;

TypeRef Feather::getCtValueType(Node* ctVal)
{
    if ( ctVal->nodeKind != nkFeatherExpCtValue )
        REP_INTERNAL(ctVal->location, "Invalid CtValue");
    return Nest_getCheckPropertyType(ctVal, "valueType");
}

/// Getter for the value data of a CtValue node -- the data is encoded in a string
const string& Feather::getCtValueDataAsString(Node* ctVal)
{
    if ( ctVal->nodeKind != nkFeatherExpCtValue )
        REP_INTERNAL(ctVal->location, "Invalid CtValue");
    return Nest_getCheckPropertyString(ctVal, "valueData");
}

bool Feather::getBoolCtValue(Node* ctVal)
{
    return (0 != *getCtValueData<unsigned char>(ctVal));
}

bool Feather::sameCtValue(Node* ctVal1, Node* ctVal2)
{
    return getCtValueType(ctVal1) == getCtValueType(ctVal2)
        && getCtValueDataAsString(ctVal1) == getCtValueDataAsString(ctVal2);
}
