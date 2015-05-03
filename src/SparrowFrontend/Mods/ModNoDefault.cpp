#include <StdInc.h>
#include "ModNoDefault.h"

#include <Nodes/SprProperties.h>

#include <Nest/Common/Diagnostic.h>

using namespace SprFrontend;

void ModNoDefault::beforeComputeType(Node* node)
{
    /// Check to apply only to classes or functions
    if ( node->nodeKind() != nkSparrowDeclSprFunction && node->nodeKind() != nkSparrowDeclSprClass )
        REP_INTERNAL(node->location(), "noDefault modifier can be applied only to classes or methods");

    node->setProperty(propNoDefault, 1);
}
