#include <StdInc.h>
#include "MemLoad.h"
#include <Feather/FeatherNodeCommonsCpp.h>

#include <Util/TypeTraits.h>


MemLoad::MemLoad(const Location& loc, DynNode* arg, size_t alignment, bool isVolatile, AtomicOrdering ordering, bool singleThreaded)
    : DynNode(classNodeKind(), loc, {arg})
{
    setProperty("alignment", alignment);
    setProperty("volatile", isVolatile ? 1 : 0);
    setProperty("atomicOrdering", (int) ordering);
    setProperty("singleThreaded", singleThreaded ? 1 : 0);
}

DynNode* MemLoad::argument() const
{
    return (DynNode*) data_.children[0];
}

size_t MemLoad::alignment() const
{
    return getCheckPropertyInt("alignment");
}

bool MemLoad::isVolatile() const
{
    return 0 != getCheckPropertyInt("volatile");
}

AtomicOrdering MemLoad::atomicOrdering() const
{
    return (AtomicOrdering) getCheckPropertyInt("atomicOrdering");
}

bool MemLoad::isSingleThread() const
{
    return getCheckPropertyInt("singleThreaded");
}


void MemLoad::dump(ostream& os) const
{
    os << "memLoad(" << data_.children[0] << ")";
}

void MemLoad::doSemanticCheck()
{
    ASSERT(data_.children[0]);

    // Semantic check the argument
    Nest::semanticCheck(data_.children[0]);

    // Check if the type of the argument is a ref
    if ( !data_.children[0]->type->hasStorage || data_.children[0]->type->numReferences == 0 )
        REP_ERROR(data_.location, "Cannot load from a non-reference (%1%, type: %2%)") % data_.children[0] % data_.children[0]->type;

    // Check flags
    AtomicOrdering ordering = atomicOrdering();
    if ( ordering == atomicRelease )
        REP_ERROR(data_.location, "Cannot use atomic release with a load instruction");
    if ( ordering == atomicAcquireRelease )
        REP_ERROR(data_.location, "Cannot use atomic acquire-release with a load instruction");

    // Remove the 'ref' from the type and get the base type
    data_.type = removeRef(data_.children[0]->type);
    data_.type = adjustMode(data_.type, data_.context, data_.location);
}

