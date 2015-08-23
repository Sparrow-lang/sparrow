#include <StdInc.h>
#include "ModCtGeneric.h"

#include <Nodes/SprProperties.h>
#include <Feather/Util/Decl.h>

#include <Nest/Common/Diagnostic.h>

using namespace SprFrontend;

void ModCtGeneric::beforeComputeType(Node* node)
{
    /// Check to apply only to classes or functions
    if ( node->nodeKind != nkSparrowDeclSprFunction )
        REP_ERROR(node->location, "ctGeneric modifier can be applied only to functions");

    Feather::setEvalMode(node, modeCt);
    Nest::setProperty(node, propCtGeneric, 1);
}
