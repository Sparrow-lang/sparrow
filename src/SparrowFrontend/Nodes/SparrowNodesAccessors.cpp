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

void SprFrontend::Class_addChild(Node* cls, Node* child)
{
    if ( !child )
        return;
    if ( childrenContext(cls) )
        Nest::setContext(child, childrenContext(cls));
    if ( cls->type )
        Nest::computeType(child);
    if ( !cls->children[2] )
    {
        cls->children[2] = mkNodeList(cls->location, {});
        if ( childrenContext(cls) )
            Nest::setContext(cls->children[2], childrenContext(cls));
    }
    cls->children[2]->children.push_back(child);
}
