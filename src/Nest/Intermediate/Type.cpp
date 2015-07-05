#include <StdInc.h>
#include "Type.h"
#include "Node.h"
#include <Common/Ser/OutArchive.h>
#include <Common/Ser/InArchive.h>

using namespace Nest;
using namespace Nest::Common::Ser;


namespace
{
    /// Get the hash code for the content of the type
    size_t getContentHash(const Type* type) noexcept
    {
        size_t res = (size_t) type->typeId
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
    bool res = lhs.typeId == rhs.typeId
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

Type* Nest::getStockType(const Type& reference)
{
    // Search the type in our stock; if not found, insert it
    // Return the type from the stock
    auto p = allTypes.insert(reference);
    return const_cast<Type*>(&*p.first); // TODO: remove the const cast
}

void Type::save(OutArchive& ar) const
{
    // TODO: serialize this
    ar.write("typeId", typeId);
    ar.write("mode", (char) mode);
    ar.write("numSubtypes", numSubtypes);
    ar.write("numReferences", numReferences);
    ar.write("hasStorage", hasStorage);
    ar.write("canBeUsedAtCt", canBeUsedAtCt);
    ar.write("canBeUsedAtRt", canBeUsedAtRt);
    ar.write("flags", flags);
    ar.write("referredNode", referredNode);
    ar.write("description", description);
    // ar.writeArray("subTypes", subTypes, [] (OutArchive& ar, Type* t) {
    //     ar.write("", t);
    // });
    // TODO: subtypes
}

void Type::load(InArchive& ar)
{
    // TODO
}