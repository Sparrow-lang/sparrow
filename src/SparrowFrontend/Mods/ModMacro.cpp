#include <StdInc.h>
#include "ModMacro.h"

#include <Nodes/SprProperties.h>

#include <Nest/Common/Diagnostic.h>

using namespace SprFrontend;

void ModMacro::beforeComputeType(DynNode* node)
{
    /// Check to apply only to functions
    if ( node->nodeKind() != nkSparrowDeclSprFunction )
        REP_ERROR(node->location(), "macro modifier can be applied only to functions");
    
    node->setProperty(propMacro, 1);
    node->setProperty(propCtGeneric, 1);
}
