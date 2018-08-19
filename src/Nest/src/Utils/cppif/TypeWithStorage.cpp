#include "Nest/src/StdInc.hpp"
#include "Nest/Utils/cppif/TypeWithStorage.hpp"
#include "Nest/Utils/Diagnostic.hpp"

namespace Nest {

TypeWithStorage::TypeWithStorage(TypeRef type)
    : Type(type) {
    if (type && !type->hasStorage)
        REP_INTERNAL(NOLOC, "TypeWithStorage constructed with non-storage type (%1%") % type;
}

} // namespace Nest
