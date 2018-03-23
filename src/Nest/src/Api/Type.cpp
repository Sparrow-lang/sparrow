#include "Nest/src/StdInc.hpp"
#include "Nest/Api/Type.h"
#include "Nest/Api/TypeKindRegistrar.h"

namespace {
/// Get the hash code for the content of the type
size_t getContentHash(const Type* type) noexcept {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    auto refNodeHash = reinterpret_cast<size_t>(type->referredNode);
    size_t res = (size_t)type->typeKind + (size_t)type->mode + type->numSubtypes +
                 type->numReferences + type->hasStorage + type->canBeUsedAtCt +
                 type->canBeUsedAtRt + type->flags + refNodeHash;
    for (int i = 0; i < type->numSubtypes; ++i)
        res += getContentHash(type->subTypes[i]);
    return res;
}

struct TypeHasher {
    size_t operator()(const Type& type) const noexcept { return getContentHash(&type); }
};

/// The set of all the types that we have registered
unordered_set<Type, TypeHasher> allTypes;
} // namespace

// Compare types by content
bool operator==(const Type& lhs, const Type& rhs) {
    bool res = lhs.typeKind == rhs.typeKind && lhs.mode == rhs.mode &&
               lhs.numSubtypes == rhs.numSubtypes && lhs.numReferences == rhs.numReferences &&
               lhs.hasStorage == rhs.hasStorage && lhs.canBeUsedAtCt == rhs.canBeUsedAtCt &&
               lhs.canBeUsedAtRt == rhs.canBeUsedAtRt && lhs.flags == rhs.flags &&
               lhs.referredNode == rhs.referredNode;
    if (!res)
        return false;
    for (int i = 0; i < lhs.numSubtypes; ++i)
        if (lhs.subTypes[i] != rhs.subTypes[i])
            return false;
    return res;
}

TypeRef Nest_findStockType(const Type* reference) {
    auto it = allTypes.find(*reference);
    return it == allTypes.end() ? nullptr : &*it;
}

TypeRef Nest_insertStockType(const Type* newType) {
    auto p = allTypes.insert(*newType);
    return &*p.first;
}

TypeRef Nest_changeTypeMode(TypeRef type, EvalMode newMode) {
    return Nest_getChangeTypeModeFun(type->typeKind)(type, newMode);
}
