#include <StdInc.h>
#include "Type.h"
#include "TypeKindRegistrar.h"
#include "NodeSer.h"
#include <Common/Ser/OutArchive.h>
#include <Common/Ser/InArchive.h>

using namespace Nest;
using namespace Nest::Common::Ser;


namespace
{
    /// Get the hash code for the content of the type
    size_t getContentHash(const Type* type) noexcept
    {
        size_t res = (size_t) type->typeKind
            + (size_t) type->mode
            + type->numSubtypes
            + type->numReferences
            + type->hasStorage
            + type->canBeUsedAtCt
            + type->canBeUsedAtRt
            + type->flags
            + reinterpret_cast<size_t>(type->referredNode)
            ;
        for ( int i=0; i< type->numSubtypes; ++i )
            res += getContentHash(type->subTypes[i]);
            return res;
    }

    struct TypeHasher
    {
        size_t operator()(const Type& type)
        {
            return getContentHash(&type);
        }
    };

    /// The set of all the types that we have registered
    unordered_set<Type, TypeHasher> allTypes;
}

bool Nest::operator ==(const Type& lhs, const Type& rhs)
{
    bool res = lhs.typeKind == rhs.typeKind
        && lhs.mode == rhs.mode
        && lhs.numSubtypes == rhs.numSubtypes
        && lhs.numReferences == rhs.numReferences
        && lhs.hasStorage == rhs.hasStorage
        && lhs.canBeUsedAtCt == rhs.canBeUsedAtCt
        && lhs.canBeUsedAtRt == rhs.canBeUsedAtRt
        && lhs.flags == rhs.flags
        && lhs.referredNode == rhs.referredNode
        ;
    if ( !res )
        return false;
    for ( int i=0; i<lhs.numSubtypes; ++i )
        if ( lhs.subTypes[i] != rhs.subTypes[i] )
            return false;
    return res;
}

TypeRef Nest::findStockType(const Type& reference)
{
    auto it = allTypes.find(reference);
    return it == allTypes.end() ? nullptr : &*it;
}

TypeRef Nest::insertStockType(const Type& newType)
{
    auto p = allTypes.insert(newType);
    return &*p.first;
}

void Nest::save(const Type& obj, OutArchive& ar)
{
    ar.write("typeKind", obj.typeKind);
    ar.write("mode", (char) obj.mode);
    ar.write("numSubtypes", obj.numSubtypes);
    ar.write("numReferences", obj.numReferences);
    ar.write("hasStorage", obj.hasStorage);
    ar.write("canBeUsedAtCt", obj.canBeUsedAtCt);
    ar.write("canBeUsedAtRt", obj.canBeUsedAtRt);
    ar.write("flags", obj.flags);
    ar.write("referredNode", obj.referredNode);
    ar.write("description", obj.description);
     ar.writeArray("subTypes", obj.subTypes, obj.subTypes+obj.numSubtypes, [] (OutArchive& ar, TypeRef t) {
         ar.write("", t);
     });
}

void Nest::load(Type& obj, InArchive& ar)
{
    // TODO
}


TypeRef Nest::changeTypeMode(TypeRef type, EvalMode newMode)
{
    return getChangeTypeModeFun(type->typeKind)(type, newMode);
}
