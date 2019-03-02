#pragma once

#include "Nest/Utils/Assert.h"

namespace Nest {

/**
 * @brief      A range of elements
 *
 * The elements in the range are considered constant; they cannot be changed.
 *
 * @tparam     T     The type of the element in the range (NodeHandle, Type, etc.)
 */
template <typename T> struct Range {
    using value_type = T;
    using iterator = const T*;
    using const_iterator = const T*;

    //! Default ctor
    Range()
        : beginPtr_(nullptr)
        , endPtr_(nullptr) {}

    //! Explicit pointer initialization
    explicit Range(const T* begin, const T* end)
        : beginPtr_(begin)
        , endPtr_(end) {}

    //! Construct from an initializer list
    explicit Range(std::initializer_list<T> l)
        : beginPtr_(l.begin())
        , endPtr_(l.end()) {}

    //! Construct from a vector
    Range(const vector<T>& l)
        : beginPtr_(&*l.begin())
        , endPtr_(&*l.end()) {}

    //! Explicit range casting
    template <typename U>
    explicit Range(Range<U> src)
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        : beginPtr_(reinterpret_cast<const T*>(src.beginPtr_))
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        , endPtr_(reinterpret_cast<const T*>(src.endPtr_)) {}

    //! Returns true if there are no elements in this range
    bool empty() const { return beginPtr_ == endPtr_; }

    //! Gets the number of elements in this range
    int size() const { return endPtr_ - beginPtr_; }
    //! Returns the element at the given index
    T operator[](int idx) const {
        ASSERT(idx < size());
        return beginPtr_[idx];
    }

    //! Skip the given number of elements and return the remaining range
    Range skip(int num) const {
        ASSERT(num >= 0);
        ASSERT(num <= size());
        return Range{beginPtr_ + num, endPtr_};
    }

    //! Shrink the range to the given size
    Range shrinkTo(int num) const {
        ASSERT(num >= 0);
        ASSERT(num <= size());
        return Range{beginPtr_, beginPtr_ + num};
    }

    const T* begin() const { return beginPtr_; }
    const T* end() const { return endPtr_; }

    //! Transform this into a vector
    vector<T> toVec() const { return vector<T>{beginPtr_, endPtr_}; }

    //! Get a string description out of this
    string toString() const {
        ostringstream ss;
        ss << *this;
        return ss.str();
    }

protected:
    const T* beginPtr_;
    const T* endPtr_;
};

template <typename T> inline ostream& operator<<(ostream& os, Range<T> r) {
    os << '[';
    bool first = true;
    for (auto el : r) {
        if (!first)
            os << ", ";
        first = false;
        os << el;
    }
    os << ']';
    return os;
}

} // namespace Nest
