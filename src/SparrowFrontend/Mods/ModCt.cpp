#include <StdInc.h>
#include "ModCt.h"

#include <Feather/Util/Decl.h>

using namespace SprFrontend;


void ModCt::beforeSetContext(Node* node)
{
    Feather::setEvalMode((DynNode*) node, Nest::modeCt);
}
