#pragma once

#include "AtomicOrdering.h"
#include <Feather/FeatherNodeCommonsH.h>

namespace Feather
{
    /// Intermediate code node that represents a memory load operation
    /// Given a ref-typed argument, this will return a type without the ref
    class MemLoad : public DynNode
    {
        DEFINE_NODE(MemLoad, nkFeatherExpMemLoad, "Feather.Exp.MemLoad");

    public:
        MemLoad(const Location& loc, DynNode* arg, size_t alignment = 0, bool isVolatile = false, AtomicOrdering ordering = atomicNone, bool singleThreaded = false);

        /// Getter for the argument of this operation
        DynNode* argument() const;

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
        void dump(ostream& os) const;

    protected:
        void doSemanticCheck();
    };
}
