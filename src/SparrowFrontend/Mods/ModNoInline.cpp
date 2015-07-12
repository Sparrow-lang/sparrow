#include <StdInc.h>
#include "ModNoInline.h"

#include <Nest/Common/Diagnostic.h>

using namespace SprFrontend;


void ModNoInline::beforeComputeType(Node* node)
{
    if ( node->nodeKind != nkSparrowDeclSprFunction )
        REP_INTERNAL(node->location, "noInline modifier can be applied only to functions");

    Nest::setProperty(node, Feather::propNoInline, 1);
}
