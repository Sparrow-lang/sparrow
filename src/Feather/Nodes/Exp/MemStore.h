#pragma once

#include "AtomicOrdering.h"
#include <Feather/FeatherNodeCommonsH.h>

namespace Feather
{
    /// Intermediate code node that represents a memory store operation
    /// Given a value and an address (reference value), the type of the address must be a ref to the type of address
    /// Note that if the address node is a VarRef, and FieldRef, this will address the content of the var/field directly.
    /// The return type will be Void
    class MemStore : public Node
    {
        DEFINE_NODE(MemStore, nkFeatherExpMemStore, "Feather.Exp.MemStore");

    public:
        MemStore(const Location& loc, Node* value, Node* address, size_t alignment = 0, bool isVolatile = false, AtomicOrdering ordering = atomicNone, bool singleThreaded = false);

        /// Getter for the value to be stored
        Node* value() const;

        /// Getter for the node that represents the address where the value to be stored
        Node* address() const;

        /// Getter for the alignment of the memory. Zero is default (backend-dependent)
        size_t alignment() const;
        
        /// Getter for the 'volatile' flag.
        /// If this flag is set, the operation is not moved relatively to other volatile operations
        bool isVolatile() const;
        
        /// Getter for the atomic ordering type
        AtomicOrdering atomicOrdering() const;
        
        /// Getter for the 'single-threaded' flag.
        /// If this flag is set, the operation is intended for single-threaded use
        bool isSingleThread() const;
        
    public:
        virtual void dump(ostream& os) const;

    protected:
        virtual void doSemanticCheck();
    };
}
