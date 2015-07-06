#include <StdInc.h>
#include "TypeKindRegistrar.h"

using namespace Nest;

namespace
{
    /// The number of registered type kinds
    static int numRegisteredTypeKinds = 0;
}

int Nest::registerTypeKind()
{
    return ++numRegisteredTypeKinds;
}
