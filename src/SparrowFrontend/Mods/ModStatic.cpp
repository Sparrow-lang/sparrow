#include <StdInc.h>
#include "ModStatic.h"

#include <Nest/Common/Diagnostic.h>

using namespace SprFrontend;


void ModStatic::beforeComputeType(DynNode* node)
{
    if ( node->nodeKind() != nkSparrowDeclSprVariable && node->nodeKind() != nkSparrowDeclSprFunction )
        REP_INTERNAL(node->location(), "Static modifier can be applied only to variables and functions inside classes");

    node->setProperty(propIsStatic, 1);
}
