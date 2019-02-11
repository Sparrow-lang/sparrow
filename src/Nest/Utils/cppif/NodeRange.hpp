#pragma once

#include "Nest/Api/NodeRange.h"
#include "Nest/Utils/cppif/NodeHandle.hpp"
#include "Nest/Utils/cppif/Range.hpp"

namespace Nest {

using Node = Nest_Node;

/**
 * @brief   A range of nodes, contiguously aligned in memory.
 *
 * This essentially represents a subset of an array of node pointers.
 * The nodes pointed by this range are mutable. However the actual pointers to the node are not.
 *
 * @see     NodeRangeM
 */
struct NodeRange {
    Nest_NodeRange range;

    NodeRange()
        : range{} {}

    //! Construct a from Nest_NodeRange data structure
    NodeRange(Nest_NodeRange r)
        : range(r) {}

    NodeRange(Nest_NodeRangeM r)
        : range{r.beginPtr, r.endPtr} {}

    //! Construct from an initializer list
    explicit NodeRange(std::initializer_list<Node*> l)
        : range{l.begin(), l.end()} {}

    //! Construct from a vector of nodes
    template <typename T>
    NodeRange(const vector<T>& l)
        : range{toRaw(&*l.begin()), toRaw(&*l.end())} {}

    //! Automatic conversion to Nest_NodeRange
    operator Nest_NodeRange() const { return range; }

    //! Returns true if there are no nodes in this range
    bool empty() const { return range.beginPtr == range.endPtr; }

    //! Gets the number of nodes in this range
    int size() const { return Nest_nodeRangeSize(range); }
    //! Returns the node handle at the given index
    NodeHandle operator[](int idx) const {
        ASSERT(idx < size());
        return range.beginPtr[idx];
    }

    //! Skip the given number of elements and return the remaining range
    NodeRange skip(int num) const {
        ASSERT(num >= 0);
        ASSERT(num <= size());
        return Nest_NodeRange{range.beginPtr + num, range.endPtr};
    }

    //! Shrink the node range to the given size
    NodeRange shrinkTo(int num) const {
        ASSERT(num >= 0);
        ASSERT(num <= size());
        return Nest_NodeRange{range.beginPtr, range.beginPtr + num};
    }

    const NodeHandle* begin() const {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        return reinterpret_cast<const NodeHandle*>(range.beginPtr);
    }
    const NodeHandle* end() const {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        return reinterpret_cast<const NodeHandle*>(range.endPtr);
    }

    vector<NodeHandle> toVec() const;

    const char* toString() const;

private:
    template <typename T> Nest_Node* const* toRaw(const T* x) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        return reinterpret_cast<Nest_Node* const*>(x);
    }
};

// TODO: unify the range classes

template <typename T> struct NodeRangeT : Range<T> {
    using NodeType = T;
    using BaseType = Range<T>;

    //! Explicit pointer initialization
    explicit NodeRangeT(T* begin, T* end)
        : BaseType(begin, end) {}

    //! Construct a from Nest_NodeRange data structure
    explicit NodeRangeT(Nest_NodeRange r)
        : BaseType(fromRaw(r.beginPtr), fromRaw(r.endPtr)) {}
    explicit NodeRangeT(Nest_NodeRangeM r)
        : BaseType(r.beginPtr, r.endPtr) {}

    //! Construct from an initializer list
    explicit NodeRangeT(std::initializer_list<T> l)
        : BaseType(l.begin(), l.end()) {}

    //! Construct from a vector of nodes
    NodeRangeT(const vector<T>& l)
        : BaseType(l) {}

    //! Explicit range casting
    template <typename U>
    explicit NodeRangeT(NodeRangeT<U> src)
        : BaseType(src) {}

    //! Automatic conversion to Nest_NodeRange
    operator Nest_NodeRange() const { return Nest_NodeRange{toRaw(BaseType::beginPtr_), toRaw(BaseType::endPtr_)}; }

private:
    static const T* fromRaw(Nest_Node* const* x) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        return reinterpret_cast<const T*>(x);
    }
    static Nest_Node* const* toRaw(const T* x) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        return reinterpret_cast<Nest_Node* const*>(x);
    }
};

/**
 * @brief   A range of nodes, contiguously aligned in memory -- with mutability.
 *
 * This essentially represents a subset of an array of node pointers.
 * The nodes pointers and the nodes contents are all mutable.
 *
 * @see     NodeRange
 */
struct NodeRangeM {
    Nest_NodeRangeM range;

    //! Construct a from Nest_NodeRangeM data structure
    NodeRangeM(Nest_NodeRangeM r)
        : range(r) {}

    //! Automatic conversion to Nest_NodeRangeM
    operator Nest_NodeRangeM() const { return range; }

    //! Automatic conversion to NodeRange
    operator NodeRange() const { return Nest_NodeRange{range.beginPtr, range.endPtr}; }

    //! Returns true if there are no nodes in this range
    bool empty() const { return range.beginPtr == range.endPtr; }

    //! Gets the number of nodes in this range
    int size() const { return (int)Nest_nodeRangeMSize(range); }
    //! Returns the node handle at the given index
    NodeHandle& operator[](int idx) const {
        ASSERT(idx < size());
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        return *reinterpret_cast<NodeHandle*>(&range.beginPtr[idx]);
    }

    const NodeHandle* begin() const {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        return reinterpret_cast<const NodeHandle*>(range.beginPtr);
    }
    const NodeHandle* end() const {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        return reinterpret_cast<const NodeHandle*>(range.endPtr);
    }

    vector<NodeHandle> toVec() const;

    const char* toString() const;
};

ostream& operator<<(ostream& os, NodeRange r);

} // namespace Nest
