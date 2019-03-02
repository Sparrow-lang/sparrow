#pragma once

#include "SparrowFrontend/NodeCommonsH.h"

namespace SprFrontend {

class Callable;

//! A vector of callables.
//! Provides deletion destructor; otherwise just a simple vector
struct Callables {
    using _VecType = vector<Callable*>;
    using value_type = _VecType::value_type;
    using iterator = _VecType::iterator;
    using const_iterator = _VecType::const_iterator;

    Callables() {}
    ~Callables();

    iterator begin() { return callables_.begin(); }
    iterator end() { return callables_.end(); }
    const_iterator begin() const { return callables_.begin(); }
    const_iterator end() const { return callables_.end(); }

    bool empty() const { return callables_.empty(); }
    int size() const { return int(callables_.size()); }
    Callable* operator[](int idx) { return callables_[idx]; }
    const Callable* operator[](int idx) const { return callables_[idx]; }

    vector<Callable*> callables_;
};

} // namespace SprFrontend
