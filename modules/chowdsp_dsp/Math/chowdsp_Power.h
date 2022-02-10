#pragma once

#include <type_traits>

/** Useful methods for computing the power function for integer exponents in the range [0, 16] */
namespace chowdsp::Power
{
// Template specialization of exponent methods using
// Addition-chain exponentiation (https://en.wikipedia.org/wiki/Addition-chain_exponentiation).

/** Optimized integer power method. */

template <int exp, typename T>
constexpr typename std::enable_if<exp == 0, T>::type
    ipow (T) noexcept
{
    return (T) 1;
}

template <int exp, typename T>
constexpr typename std::enable_if<exp == 1, T>::type
    ipow (T a) noexcept
{
    return a;
}

template <int exp, typename T>
constexpr typename std::enable_if<exp == 2, T>::type
    ipow (T a) noexcept
{
    return a * a;
}

template <int exp, typename T>
constexpr typename std::enable_if<exp == 3, T>::type
    ipow (T a) noexcept
{
    return a * a * a;
}

template <int exp, typename T>
constexpr typename std::enable_if<exp == 4, T>::type
    ipow (T a) noexcept
{
    const auto b = ipow<2> (a);
    return ipow<2> (b);
}

template <int exp, typename T>
constexpr typename std::enable_if<exp == 5, T>::type
    ipow (T a) noexcept
{
    return ipow<4> (a) * a;
}

template <int exp, typename T>
constexpr typename std::enable_if<exp == 6, T>::type
    ipow (T a) noexcept
{
    const auto b = ipow<2> (a);
    return ipow<3> (b);
}

template <int exp, typename T>
constexpr typename std::enable_if<exp == 7, T>::type
    ipow (T a) noexcept
{
    return ipow<6> (a) * a;
}

template <int exp, typename T>
constexpr typename std::enable_if<exp == 8, T>::type
    ipow (T a) noexcept
{
    const auto d = ipow<4> (a);
    return ipow<2> (d);
}

template <int exp, typename T>
constexpr typename std::enable_if<exp == 9, T>::type
    ipow (T a) noexcept
{
    const auto c = ipow<3> (a);
    return ipow<3> (c);
}

template <int exp, typename T>
constexpr typename std::enable_if<exp == 10, T>::type
    ipow (T a) noexcept
{
    const auto b = ipow<2> (a);
    return ipow<4> (b) * b;
}

template <int exp, typename T>
constexpr typename std::enable_if<exp == 11, T>::type
    ipow (T a) noexcept
{
    return ipow<10> (a) * a;
}

template <int exp, typename T>
constexpr typename std::enable_if<exp == 12, T>::type
    ipow (T a) noexcept
{
    const auto d = ipow<4> (a);
    return ipow<3> (d);
}

template <int exp, typename T>
constexpr typename std::enable_if<exp == 13, T>::type
    ipow (T a) noexcept
{
    return ipow<12> (a) * a;
}

template <int exp, typename T>
constexpr typename std::enable_if<exp == 14, T>::type
    ipow (T a) noexcept
{
    const auto b = ipow<2> (a);
    return ipow<6> (b) * b;
}

template <int exp, typename T>
constexpr typename std::enable_if<exp == 15, T>::type
    ipow (T a) noexcept
{
    const auto e = ipow<5> (a);
    return ipow<3> (e);
}

template <int exp, typename T>
constexpr typename std::enable_if<exp == 16, T>::type
    ipow (T a) noexcept
{
    const auto h = ipow<8> (a);
    return ipow<2> (h);
}
} // namespace chowdsp::Power
