#pragma once

#include <tuple>

namespace chowdsp
{
/** Helpers for working with std::tuple */
namespace TupleHelpers
{
    /** Do a function for each element in the tuple */
    template <typename Fn, typename Tuple, size_t... Ix>
    constexpr void forEachInTuple (Fn&& fn, Tuple&& tuple, std::index_sequence<Ix...>) noexcept (noexcept (std::initializer_list<int> { (fn (std::get<Ix> (tuple), std::integral_constant<std::size_t, Ix>()), 0)... }))
    {
        (void) std::initializer_list<int> { ((void) fn (std::get<Ix> (std::forward<Tuple> (tuple)), std::integral_constant<std::size_t, Ix>()), 0)... };
    }

    template <typename T>
    using TupleIndexSequence = std::make_index_sequence<std::tuple_size<std::remove_cv_t<std::remove_reference_t<T>>>::value>;

    /** Do a function for each element in the tuple */
    template <typename Fn, typename Tuple>
    constexpr void forEachInTuple (Fn&& fn, Tuple&& tuple) noexcept (noexcept (forEachInTuple (std::forward<Fn> (fn), std::forward<Tuple> (tuple), TupleIndexSequence<Tuple> {})))
    {
        forEachInTuple (std::forward<Fn> (fn), std::forward<Tuple> (tuple), TupleIndexSequence<Tuple> {});
    }

#ifndef DOXYGEN
    namespace tuple_visit_detail
    {
        template <size_t I>
        struct visit_impl
        {
            template <typename T, typename F>
            static void visit (T& tup, size_t idx, F&& fun)
            {
                if (idx == I - 1)
                    fun (std::get<I - 1> (tup));
                else
                    visit_impl<I - 1>::visit (tup, idx, std::forward<F> (fun));
            }
        };

        template <>
        struct visit_impl<0>
        {
            template <typename T, typename F>
            static void visit (T&, size_t, F&&)
            {
                // Attempting to visit tuple at an invalid index!
                jassertfalse;
            }
        };
    } // namespace tuple_visit_detail
#endif

    /** Visits the member of the tuple at the given index */
    template <typename F, typename... Ts>
    void visit_at (const std::tuple<Ts...>& tup, size_t idx, F&& fun)
    {
        tuple_visit_detail::visit_impl<sizeof...(Ts)>::visit (tup, idx, std::forward<F> (fun));
    }

    /** Visits the member of the tuple at the given index */
    template <typename F, typename... Ts>
    void visit_at (std::tuple<Ts...>& tup, size_t idx, F&& fun)
    {
        tuple_visit_detail::visit_impl<sizeof...(Ts)>::visit (tup, idx, std::forward<F> (fun));
    }
} // namespace TupleHelpers
} // namespace chowdsp
