#include <StdInc.h>
#include "This.h"
#include <NodeCommonsCpp.h>

using namespace SprFrontend;

This::This(const Location& loc)
    : DynNode(classNodeKind(), loc)
{
}

void This::doSemanticCheck()
{
    setExplanation(mkIdentifier(data_.location, "$this"));
}
