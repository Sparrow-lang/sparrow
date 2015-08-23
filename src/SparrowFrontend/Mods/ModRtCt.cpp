#include <StdInc.h>
#include "ModRtCt.h"

#include <Feather/Util/Decl.h>

using namespace SprFrontend;


void ModRtCt::beforeSetContext(Node* node)
{
    Feather::setEvalMode(node, modeRtCt);
}
