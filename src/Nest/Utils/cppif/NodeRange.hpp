#pragma once

#include "Nest/Api/NodeRange.h"
#include "Nest/Utils/cppif/NodeHandle.hpp"

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

template <typename T> struct NodeRangeT {
    using NodeType = T;

    //! Explicit pointer initialization
    explicit NodeRangeT(T* begin, T* end)
        : beginPtr_(begin)
        , endPtr_(end) {}

    //! Construct a from Nest_NodeRange data structure
    explicit NodeRangeT(Nest_NodeRange r)
        : beginPtr_(fromRaw(r.beginPtr))
        , endPtr_(fromRaw(r.endPtr)) {}
    explicit NodeRangeT(Nest_NodeRangeM r)
        : beginPtr_(r.beginPtr)
        , endPtr_(r.endPtr) {}

    //! Construct from an initializer list
    explicit NodeRangeT(std::initializer_list<T> l)
        : beginPtr_(l.begin())
        , endPtr_(l.end()) {}

    //! Construct from a vector of nodes
    NodeRangeT(const vector<T>& l)
        : beginPtr_(&*l.begin())
        , endPtr_(&*l.end()) {}

    //! Explicit range casting
    template <typename U>
    explicit NodeRangeT(NodeRangeT<U> src)
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        : beginPtr_(reinterpret_cast<T*>(src.beginPtr_))
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        , endPtr_(reinterpret_cast<T*>(src.endPtr_)) {}

    //! Automatic conversion to Nest_NodeRange
    operator Nest_NodeRange() const { return Nest_NodeRange{toRaw(beginPtr_), toRaw(endPtr_)}; }

    //! Returns true if there are no nodes in this range
    bool empty() const { return beginPtr_ == endPtr_; }

    //! Gets the number of nodes in this range
    int size() const { return endPtr_ - beginPtr_; }
    //! Returns the node handle at the given index
    T operator[](int idx) const {
        ASSERT(idx < size());
        return beginPtr_[idx];
    }

    //! Skip the given number of elements and return the remaining range
    NodeRangeT skip(int num) const {
        ASSERT(num >= 0);
        ASSERT(num <= size());
        return NodeRangeT{beginPtr_ + num, endPtr_};
    }

    //! Shrink the node range to the given size
    NodeRangeT shrinkTo(int num) const {
        ASSERT(num >= 0);
        ASSERT(num <= size());
        return NodeRangeT{beginPtr_, beginPtr_ + num};
    }

    const T* begin() const { return beginPtr_; }
    const T* end() const { return endPtr_; }

    //! Transform this into a vector
    vector<T> toVec() const { return vector<T>{beginPtr_, endPtr_}; }

    //! Get a string description out of this
    const char* toString() const {
        ostringstream ss;
        ss << *this;
        return strdup(ss.str().c_str());
    }

private:
    static const T* fromRaw(Nest_Node* const* x) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        return reinterpret_cast<const T*>(x);
    }
    static Nest_Node* const* toRaw(const T* x) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        return reinterpret_cast<Nest_Node* const*>(x);
    }

    const T* beginPtr_;
    const T* endPtr_;
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
