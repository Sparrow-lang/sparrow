#include "Nest/src/StdInc.hpp"
#include "Nest/Utils/cppif/Type.hpp"
#include "Nest/Utils/Diagnostic.hpp"

namespace Nest {

Type Type::changeMode(EvalMode mode, Location loc) {
    if (mode == type_->mode)
        return *this;

    TypeRef resType = Nest_changeTypeMode(type_, mode);
    if (!resType)
        REP_INTERNAL(loc, "Don't know how to change eval mode of type %1%") % type_;
    ASSERT(resType);

    if (mode == modeCt && resType->mode != modeCt)
        REP_ERROR_RET(nullptr, loc, "Type '%1%' cannot be used at compile-time") % type_;

    return resType;
}

bool sameTypeIgnoreMode(Type t1, Type t2) {
    ASSERT(t1);
    ASSERT(t2);
    if (t1 == t2)
        return true;
    if (t1.kind() != t2.kind() || t1.mode() == t2.mode())
        return false;
    auto t = t1.changeMode(t2.mode());
    return t == t2;
}

ostream& operator<<(ostream& os, Type type) { return os << type.description(); }

} // namespace Nest
