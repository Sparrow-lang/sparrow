#include <StdInc.h>
#include "MemStore.h"
#include <Feather/FeatherNodeCommonsCpp.h>

#include <Util/TypeTraits.h>


MemStore::MemStore(const Location& loc, Node* value, Node* address, size_t alignment, bool isVolatile, AtomicOrdering ordering, bool singleThreaded)
    : Node(classNodeKind(), loc, {value, address})
{
    setProperty("alignment", alignment);
    setProperty("volatile", isVolatile ? 1 : 0);
    setProperty("atomicOrdering", (int) ordering);
    setProperty("singleThreaded", singleThreaded ? 1 : 0);
}

Node* MemStore::value() const
{
    return children_[0];
}

Node* MemStore::address() const
{
    return children_[1];
}

size_t MemStore::alignment() const
{
    return getCheckPropertyInt("alignment");
}

bool MemStore::isVolatile() const
{
    return 0 != getCheckPropertyInt("volatile");
}

AtomicOrdering MemStore::atomicOrdering() const
{
    return (AtomicOrdering) getCheckPropertyInt("atomicOrdering");
}

bool MemStore::isSingleThread() const
{
    return getCheckPropertyInt("singleThreaded");
}


void MemStore::dump(ostream& os) const
{
    os << "memStore(" << value() << ", " << address() << ")";
}

void MemStore::doSemanticCheck()
{
    Node* value = children_[0];
    Node* address = children_[1];
    ASSERT(value);
    ASSERT(address);

    // Semantic check the arguments
    value->semanticCheck();
    address->semanticCheck();

    // Check if the type of the address is a ref
    if ( !address->type()->hasStorage || address->type()->numReferences == 0 )
        REP_ERROR(location_, "The address of a memory store is not a reference, nor VarRef nor FieldRef (type: %1%)") % address->type();
    TypeRef baseAddressType = removeRef(address->type());

    // Check the equivalence of types
    if ( !isSameTypeIgnoreMode(value->type(), baseAddressType) )
    {
        // Try again, getting rid of l-values
        TypeRef t1 = lvalueToRefIfPresent(value->type());
        if ( !isSameTypeIgnoreMode(t1, baseAddressType) )
            REP_ERROR(location_, "The type of the value doesn't match the type of the address in a memory store (%1% != %2%)")
                % value->type() % baseAddressType;
    }

    // The resulting type is Void
    type_ = getVoidType(address->type()->mode);
}

