#include <StdInc.h>
#include "TypeKindRegistrar.h"
#include "Type.h"

using namespace Nest;

namespace
{
    struct TypeFunctions
    {
        FChangeTypeMode changeTypeMode;
    };

    /// The registered type kinds
    static vector<TypeFunctions> registeredTypeKinds;
}

int Nest::registerTypeKind(FChangeTypeMode funChangeTypeMode)
{
    int typeKindId = registeredTypeKinds.size();
    TypeFunctions f = { funChangeTypeMode };
    registeredTypeKinds.push_back(f);
    return typeKindId;
}

FChangeTypeMode Nest::getChangeTypeModeFun(int typeKind)
{
    ASSERT(0 <= typeKind  && typeKind < registeredTypeKinds.size());
    return registeredTypeKinds[typeKind].changeTypeMode;
}
