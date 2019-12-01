#pragma once

namespace _Impl {
template <typename T> struct reversion_wrapper { T& iterable; };

template <typename T> auto begin(reversion_wrapper<T> w) { return std::rbegin(w.iterable); }

template <typename T> auto end(reversion_wrapper<T> w) { return std::rend(w.iterable); }
} // namespace _Impl

//! Reverses the given iterable. To be used in ranged-for constructs
template <typename T> _Impl::reversion_wrapper<T> reverse(T&& iterable) { return {iterable}; }