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
    size_t getContentHash(const TypeData* type) noexcept
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
            res += getContentHash(&type->subTypes[i]->data_);
            return res;
    }

    struct TypeHasher
    {
        size_t operator()(const TypeData& type)
        {
            return getContentHash(&type);
        }
    };

    /// The set of all the types that we have registered
    unordered_set<TypeData, TypeHasher> allTypes;
}

bool Nest::operator ==(const TypeData& lhs, const TypeData& rhs)
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

TypeData* Nest::getStockType(const TypeData& reference)
{
    // Search the type in our stock; if not found, insert it
    // Return the type from the stock
    auto p = allTypes.insert(reference);
    return const_cast<TypeData*>(&*p.first); // TODO: remove the const cast
}

Type::Type(unsigned typeId, EvalMode mode)
{
    data_.typeId        = typeId;
    data_.mode          = mode;
    data_.numSubtypes   = 0;
    data_.numReferences = 0;
    data_.hasStorage    = 0;
    data_.canBeUsedAtCt = 1;
    data_.canBeUsedAtRt = 1;
    data_.flags         = 0;
    data_.referredNode  = nullptr;
    data_.description   = "";
}


void Type::save(OutArchive& ar) const
{
    // TODO: serialize this
    ar.write("typeId", data_.typeId);
    ar.write("mode", (char) data_.mode);
    ar.write("numSubtypes", data_.numSubtypes);
    ar.write("numReferences", data_.numReferences);
    ar.write("hasStorage", data_.hasStorage);
    ar.write("canBeUsedAtCt", data_.canBeUsedAtCt);
    ar.write("canBeUsedAtRt", data_.canBeUsedAtRt);
    ar.write("flags", data_.flags);
    ar.write("referredNode", data_.referredNode);
    ar.write("description", data_.description);
    // ar.writeArray("subTypes", data_.subTypes, [] (OutArchive& ar, TypeData* t) {
    //     ar.write("", t);
    // });
    // TODO: subtypes
}

void Type::load(InArchive& ar)
{
    // TODO
}