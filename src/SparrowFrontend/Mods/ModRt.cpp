#include <StdInc.h>
#include "ModRt.h"

#include <Feather/Util/Decl.h>

using namespace SprFrontend;


void ModRt::beforeSetContext(Node* node)
{
    Feather::setEvalMode(node, modeRt);
}
