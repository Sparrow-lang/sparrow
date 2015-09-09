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
        Nest_computeType(child);    // Ignore possible errors
    Node*& members = at(cls->children, 2);
    if ( !members )
    {
        members = mkNodeList(cls->location, {});
        if ( Nest_childrenContext(cls) )
            Nest_setContext(members, Nest_childrenContext(cls));
    }
    Nest_appendNodeToArray(&members->children, child);
}
