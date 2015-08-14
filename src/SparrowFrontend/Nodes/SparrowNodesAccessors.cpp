#include <StdInc.h>
#include "SparrowNodesAccessors.h"

using namespace SprFrontend;
using namespace Feather;
using namespace Nest;


bool SprFrontend::Literal_isString(Node* node)
{
    const string& type = getCheckPropertyString(node, "spr.literalType"); 
    return type == "StringRef";
}

string SprFrontend::Literal_getData(Node* node)
{
    ASSERT(node->nodeKind == nkSparrowExpLiteral);
    return getCheckPropertyString(node, "spr.literalData");
}
