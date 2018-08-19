#include "Nest/src/StdInc.hpp"
#include "Nest/Api/Type.h"
#include "Nest/Api/TypeKindRegistrar.h"

namespace {
/// Get the hash code for the content of the type
size_t getContentHash(Nest_TypeRef type) noexcept {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    auto refNodeHash = reinterpret_cast<size_t>(type->referredNode);
    size_t res = (size_t)type->typeKind + (size_t)type->mode + type->numSubtypes +
                 type->numReferences + type->hasStorage + type->canBeUsedAtRt + type->flags +
                 refNodeHash;
    for (int i = 0; i < type->numSubtypes; ++i)
        res += getContentHash(type->subTypes[i]);
    return res;
}

struct TypeHasher {
    size_t operator()(const Nest_Type& type) const noexcept { return getContentHash(&type); }
};

/// The set of all the types that we have registered
unordered_set<Nest_Type, TypeHasher> g_allTypes;
} // namespace

// Compare types by content
bool operator==(const Nest_Type& lhs, const Nest_Type& rhs) {
    bool res = lhs.typeKind == rhs.typeKind && lhs.mode == rhs.mode &&
               lhs.numSubtypes == rhs.numSubtypes && lhs.numReferences == rhs.numReferences &&
               lhs.hasStorage == rhs.hasStorage && lhs.canBeUsedAtRt == rhs.canBeUsedAtRt &&
               lhs.flags == rhs.flags && lhs.referredNode == rhs.referredNode;
    if (!res)
        return false;
    for (int i = 0; i < lhs.numSubtypes; ++i)
        if (lhs.subTypes[i] != rhs.subTypes[i])
            return false;
    return res;
}

Nest_TypeRef Nest_findStockType(Nest_TypeRef reference) {
    auto it = g_allTypes.find(*reference);
    return it == g_allTypes.end() ? nullptr : &*it;
}

Nest_TypeRef Nest_insertStockType(Nest_TypeRef newType) {
    auto p = g_allTypes.insert(*newType);
    return &*p.first;
}

Nest_TypeRef Nest_changeTypeMode(Nest_TypeRef type, EvalMode newMode) {
    return Nest_getChangeTypeModeFun(type->typeKind)(type, newMode);
}

void Nest_resetTypes() { g_allTypes.clear(); }