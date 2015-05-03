#include <StdInc.h>
#include "StorageType.h"
#include <Nodes/Properties.h>
#include <Nodes/Decls/Class.h>

#include <Nest/Common/Diagnostic.h>

using namespace Feather;
using namespace Nest;

const string* StorageType::nativeName() const
{
    Class* cls = classDecl();
    return cls ? cls->getPropertyString(propNativeName) : nullptr;
}
