#pragma once

#include "Nest/Utils/Assert.h"
#include "Nest/Utils/cppif/Range.hpp"

#include <vector>

namespace Nest {

/**
 * @brief      A basic vector for storing a small number of elements.
 *
 * If we store just a few elements, we will store them inside this object. If the number of elements
 * exceeds the given parameter, we will store them in a vector.
 *
 * Adding new elements can transition existing elements from static data to dynamic data. Removing
 * elements will not transition them to static data.
 *
 * @tparam     T            The type of the element in the vector
 * @tparam     staticSize   Number of elements to keep on the stack
 */
template <typename T, int staticSize = 7> struct SmallVector {
    using value_type = T;
    using iterator = const T*;
    using const_iterator = const T*;

    //! Default ctor
    SmallVector() {}

    //! Copy ctor
    SmallVector(const SmallVector& src) { insert(Range<T>(src)); }

    //! Initialize from a range
    SmallVector(Range<T> elems) { insert(elems); }

    //! Construct from an initializer list
    SmallVector(std::initializer_list<T> elems) { insert(Range<T>(elems)); }

    //! Construct from a vector
    SmallVector(const vector<T>& elems) { insert(Range<T>(elems)); }

    //! Returns true if the vector is small and we keep all the data on stack
    bool isSmall() const { return numStaticElems_ >= 0; }

    //! Returns true if there are no elements in this vector
    bool empty() const { return numStaticElems_ == 0; }

    //! Gets the number of elements in this vector
    int size() const { return numStaticElems_ >= 0 ? numStaticElems_ : dynamicData_.size(); }
    //! Returns the element at the given index
    const T& operator[](int idx) const {
        ASSERT(idx >= 0);
        ASSERT(idx < size());
        return isSmall() ? *staticArr(idx) : dynamicData_[idx];
    }
    T& operator[](int idx) {
        ASSERT(idx >= 0);
        ASSERT(idx < size());
        return isSmall() ? *staticArr(idx) : dynamicData_[idx];
    }

    const T* begin() const { return isSmall() ? staticArr(0) : &dynamicData_[0]; }
    T* begin() { return isSmall() ? staticArr(0) : &dynamicData_[0]; }
    const T* end() const { return isSmall() ? staticArr(numStaticElems_) : &*dynamicData_.end(); }
    T* end() { return isSmall() ? staticArr(numStaticElems_) : &*dynamicData_.end(); }

    //! Returns a range with all the elements
    Range<T> all() const { return Range<T>(begin(), end()); }

    //! Implicit conversion to range
    operator Range<T>() const { return Range<T>(begin(), end()); }

    //! Ensure we have the given number of elements in the vector
    void reserve(int sz) {
        if (sz > size())
            grow(sz - size());
    }

    //! Insert a range of elements at the end of the vector
    void insert(Range<T> elems) {
        int sz = elems.size();
        grow(sz);
        if (isSmall()) {
            T* p = staticArr(numStaticElems_);
            for (const auto& el : elems)
                ::new (p++) T(el);
            numStaticElems_ += sz;
        } else
            dynamicData_.insert(dynamicData_.end(), elems.begin(), elems.end());
    }

    //! Appends an element to the vector
    void push_back(const T& el) {
        grow(1);
        if (isSmall()) {
            ::new (staticArr(numStaticElems_)) T(el);
            numStaticElems_++;
        } else
            dynamicData_.push_back(el);
    }

    //! Pop the last element from the array
    void pop_back() {
        ASSERT(size() > 0);
        if (isSmall())
            --numStaticElems_;
        else {
            dynamicData_.pop_back();
        }
    }

    //! Get a string description out of this
    string toString() const {
        ostringstream ss;
        ss << *this;
        return ss.str();
    }

protected:
    //! The number of elements in the static data; -1 if we are using dynamic data
    int numStaticElems_{0};
    //! The static space to store the first elements in
    alignas(alignof(T)) char staticData_[staticSize * sizeof(T)];
    //! Dynamic space to store the rest of the elements; used if we have too many elements
    vector<T> dynamicData_;

    const T* staticArr(int idx) const {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        return reinterpret_cast<const T*>(staticData_) + idx;
    }
    T* staticArr(int idx) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        return reinterpret_cast<T*>(staticData_) + idx;
    }

    void grow(int numElems) {
        if (numStaticElems_ < 0) {
            dynamicData_.reserve(dynamicData_.size() + numElems);
        } else if (numStaticElems_ + numElems > staticSize) {
            // Move the elements to the dynamic data
            dynamicData_.reserve(numStaticElems_ + numElems);
            dynamicData_.insert(dynamicData_.end(), staticArr(0), staticArr(numStaticElems_));
            destroyRange(staticArr(0), staticArr(numStaticElems_));
            numStaticElems_ = -1;
        }
        // Else, nothing to do
    }

    void destroyRange(T* first, T* last) {
        while (first != last) {
            --last;
            last->~T();
        }
    }
};

template <typename T, int staticSize>
inline ostream& operator<<(ostream& os, SmallVector<T, staticSize> r) {
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
