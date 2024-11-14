#pragma once

#include <array>

namespace chowdsp
{
#ifndef DOXYGEN
namespace make_array_detail
{
    template <typename T, size_t... Is, typename... Args>
    constexpr std::array<T, sizeof...(Is)> make_array (std::index_sequence<Is...>, Args&&... args)
    {
        return { (static_cast<void> (Is), T { std::forward<Args> (args)... })... };
    }

    template <typename T, size_t... Is, typename Maker>
    constexpr std::array<T, sizeof...(Is)> make_array_lambda (std::index_sequence<Is...>, Maker&& maker)
    {
        return { (T { maker (std::integral_constant<size_t, Is>()) })... };
    }
} // namespace make_array_detail
#endif

/** Returns an array of size N, with each value initialized by calling the class constructor with the provided args. */
template <typename T, size_t N, typename... Args>
constexpr std::array<T, N> make_array (Args&&... args)
{
    return make_array_detail::make_array<T> (std::make_index_sequence<N> {},
                                             std::forward<Args> (args)...);
}

/** Returns an array of size N, with each value initialized by calling the maker lambda with signature [] (size_t index) -> T. */
template <typename T, size_t N, typename Maker>
constexpr std::array<T, N> make_array_lambda (Maker&& maker)
{
    return make_array_detail::make_array_lambda<T> (std::make_index_sequence<N> {},
                                                    std::forward<Maker> (maker));
}
} // namespace chowdsp
