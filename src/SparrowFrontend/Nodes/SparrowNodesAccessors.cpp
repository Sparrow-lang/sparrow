#include <StdInc.h>
#include "SparrowNodesAccessors.h"

using namespace SprFrontend;
using namespace Feather;
using namespace Nest;


bool SprFrontend::Literal_isString(Node* node)
{
    const string& type = Nest_getCheckPropertyString(node, "spr.literalType"); 
    return type == "StringRef";
}

string SprFrontend::Literal_getData(Node* node)
{
    ASSERT(node->nodeKind == nkSparrowExpLiteral);
    return Nest_getCheckPropertyString(node, "spr.literalData");
}

void SprFrontend::Class_addChild(Node* cls, Node* child)
{
    if ( !child )
        return;
    if ( Nest_childrenContext(cls) )
        Nest_setContext(child, Nest_childrenContext(cls));
    if ( cls->type )
        Nest_computeType(child);
    if ( !cls->children[2] )
    {
        cls->children[2] = mkNodeList(cls->location, {});
        if ( Nest_childrenContext(cls) )
            Nest_setContext(cls->children[2], Nest_childrenContext(cls));
    }
    cls->children[2]->children.push_back(child);
}
