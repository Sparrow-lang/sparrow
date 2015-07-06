#include <StdInc.h>
#include "ModRt.h"

#include <Feather/Util/Decl.h>

using namespace SprFrontend;


void ModRt::beforeSetContext(DynNode* node)
{
    Feather::setEvalMode(node, Nest::modeRt);
}
