#include "Feather/src/StdInc.h"
#include "Feather/Utils/Ct.h"
#include "Feather/Api/FeatherNodes.h"
#include "Nest/Utils/Diagnostic.hpp"
#include "Nest/Utils/StringRef.hpp"
#include "Nest/Utils/NodeUtils.h"

using namespace Feather;

TypeRef Feather::getCtValueType(Node* ctVal)
{
    if ( ctVal->nodeKind != nkFeatherExpCtValue )
        REP_INTERNAL(ctVal->location, "Invalid CtValue");
    return Nest_getCheckPropertyType(ctVal, "valueType");
}

/// Getter for the value data of a CtValue node -- the data is encoded in a string
StringRef Feather::getCtValueDataAsString(Node* ctVal)
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
