#pragma once

#include <Feather/FeatherNodeCommonsH.h>

namespace Feather
{
    /// Intermediate code node that represents the operation of dereference of a reference/pointer.
    /// The type of this node is given by the type of the argument, removing the 'ref'
    class StackAlloc : public DynNode
    {
        DEFINE_NODE(StackAlloc, nkFeatherExpStackAlloc, "Feather.Exp.StackAlloc");

    public:
        StackAlloc(const Location& loc, DynNode* elemType, int numElements = 1, size_t alignment = 0);

        /// Getter for the type of elements to be allocated
        TypeRef elemType() const;

        /// Getter/setter for the number of elements of the given type to be allocated
        size_t numElements() const;

        /// Getter/setter for the alignment of the memory. Zero is default (backend-dependent)
        size_t alignment() const;

    public:
        void dump(ostream& os) const;

    protected:
        void doSemanticCheck();
    };
}
