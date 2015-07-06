#include <StdInc.h>
#include "ModRtCt.h"

#include <Feather/Util/Decl.h>

using namespace SprFrontend;


void ModRtCt::beforeSetContext(DynNode* node)
{
    Feather::setEvalMode(node, Nest::modeRtCt);
}
