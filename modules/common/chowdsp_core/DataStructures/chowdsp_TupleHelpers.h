#pragma once

#include <tuple>

namespace chowdsp
{
/** Helpers for working with std::tuple */
namespace TupleHelpers
{
    /** Do a function for each element in the tuple */
    template <typename Fn, typename Tuple, size_t... Ix>
    constexpr void forEachInTuple (Fn&& fn, Tuple&& tuple, std::index_sequence<Ix...>) noexcept (noexcept (std::initializer_list<int> { (fn (std::get<Ix> (tuple), Ix), 0)... }))
    {
        (void) std::initializer_list<int> { ((void) fn (std::get<Ix> (tuple), Ix), 0)... };
    }

    template <typename T>
    using TupleIndexSequence = std::make_index_sequence<std::tuple_size<std::remove_cv_t<std::remove_reference_t<T>>>::value>;

    /** Do a function for each element in the tuple */
    template <typename Fn, typename Tuple>
    constexpr void forEachInTuple (Fn&& fn, Tuple&& tuple) noexcept (noexcept (forEachInTuple (std::forward<Fn> (fn), std::forward<Tuple> (tuple), TupleIndexSequence<Tuple> {})))
    {
        forEachInTuple (std::forward<Fn> (fn), std::forward<Tuple> (tuple), TupleIndexSequence<Tuple> {});
    }
} // namespace TupleHelpers
} // namespace chowdsp
