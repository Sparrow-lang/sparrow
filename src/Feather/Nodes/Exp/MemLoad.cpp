#include <StdInc.h>
#include "MemLoad.h"
#include <Feather/FeatherNodeCommonsCpp.h>

#include <Util/TypeTraits.h>


MemLoad::MemLoad(const Location& loc, Node* arg, size_t alignment, bool isVolatile, AtomicOrdering ordering, bool singleThreaded)
    : Node(loc, {arg})
{
    setProperty("alignment", alignment);
    setProperty("volatile", isVolatile ? 1 : 0);
    setProperty("atomicOrdering", (int) ordering);
    setProperty("singleThreaded", singleThreaded ? 1 : 0);
}

Node* MemLoad::argument() const
{
    return children_[0];
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
    os << "memLoad(" << children_[0] << ")";
}

void MemLoad::doSemanticCheck()
{
    ASSERT(children_[0]);

    // Semantic check the argument
    children_[0]->semanticCheck();

    // Check if the type of the argument is a ref
    if ( !children_[0]->type()->hasStorage() || children_[0]->type()->noReferences() == 0 )
        REP_ERROR(location_, "Cannot load from a non-reference (%1%, type: %2%)") % children_[0] % children_[0]->type();

    // Check flags
    AtomicOrdering ordering = atomicOrdering();
    if ( ordering == atomicRelease )
        REP_ERROR(location_, "Cannot use atomic release with a load instruction");
    if ( ordering == atomicAcquireRelease )
        REP_ERROR(location_, "Cannot use atomic acquire-release with a load instruction");

    // Remove the 'ref' from the type and get the base type
    type_ = removeRef(children_[0]->type());
    type_ = adjustMode(type_, context_, location_);
}

