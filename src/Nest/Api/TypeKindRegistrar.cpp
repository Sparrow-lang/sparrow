#include "Nest/Internal/StdInc.hpp"
#include "TypeKindRegistrar.h"
#include "Type.h"

namespace
{
    struct TypeFunctions
    {
        FChangeTypeMode changeTypeMode;
    };

    /// The registered type kinds
    static vector<TypeFunctions> registeredTypeKinds;
}

int Nest_registerTypeKind(FChangeTypeMode funChangeTypeMode)
{
    int typeKindId = registeredTypeKinds.size();
    TypeFunctions f = { funChangeTypeMode };
    registeredTypeKinds.push_back(f);
    return typeKindId;
}

FChangeTypeMode Nest_getChangeTypeModeFun(int typeKind)
{
    ASSERT(0 <= typeKind  && typeKind < registeredTypeKinds.size());
    return registeredTypeKinds[typeKind].changeTypeMode;
}
