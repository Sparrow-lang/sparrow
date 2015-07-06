#include <StdInc.h>
#include "ModCt.h"

#include <Feather/Util/Decl.h>

using namespace SprFrontend;


void ModCt::beforeSetContext(DynNode* node)
{
    Feather::setEvalMode(node, Nest::modeCt);
}
