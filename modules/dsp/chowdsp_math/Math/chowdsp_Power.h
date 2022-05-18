#pragma once

#include <type_traits>

namespace chowdsp
{
/** Useful methods for computing the power function for integer exponents in the range [0, 16] */
namespace Power
{
    // Template specialization of exponent methods using
    // Addition-chain exponentiation (https://en.wikipedia.org/wiki/Addition-chain_exponentiation).

    /** Optimized integer power method. */
    template <int exp, typename T>
    constexpr std::enable_if_t<exp == 0, T>
        ipow (T) noexcept
    {
        return (T) 1;
    }

    template <int exp, typename T>
    constexpr std::enable_if_t<exp == 1, T>
        ipow (T a) noexcept
    {
        return a;
    }

    template <int exp, typename T>
    constexpr std::enable_if_t<exp == 2, T>
        ipow (T a) noexcept
    {
        return a * a;
    }

    template <int exp, typename T>
    constexpr std::enable_if_t<exp == 3, T>
        ipow (T a) noexcept
    {
        return ipow<2> (a) * a;
    }

    template <int exp, typename T>
    constexpr std::enable_if_t<exp == 4, T>
        ipow (T a) noexcept
    {
        const auto b = ipow<2> (a);
        return ipow<2> (b);
    }

    template <int exp, typename T>
    constexpr std::enable_if_t<exp == 5, T>
        ipow (T a) noexcept
    {
        return ipow<4> (a) * a;
    }

    template <int exp, typename T>
    constexpr std::enable_if_t<exp == 6, T>
        ipow (T a) noexcept
    {
        const auto b = ipow<2> (a);
        return ipow<3> (b);
    }

    template <int exp, typename T>
    constexpr std::enable_if_t<exp == 7, T>
        ipow (T a) noexcept
    {
        return ipow<6> (a) * a;
    }

    template <int exp, typename T>
    constexpr std::enable_if_t<exp == 8, T>
        ipow (T a) noexcept
    {
        const auto d = ipow<4> (a);
        return ipow<2> (d);
    }

    template <int exp, typename T>
    constexpr std::enable_if_t<exp == 9, T>
        ipow (T a) noexcept
    {
        const auto c = ipow<3> (a);
        return ipow<3> (c);
    }

    template <int exp, typename T>
    constexpr std::enable_if_t<exp == 10, T>
        ipow (T a) noexcept
    {
        const auto e = ipow<5> (a);
        return ipow<2> (e);
    }

    template <int exp, typename T>
    constexpr std::enable_if_t<exp == 11, T>
        ipow (T a) noexcept
    {
        return ipow<10> (a) * a;
    }

    template <int exp, typename T>
    constexpr std::enable_if_t<exp == 12, T>
        ipow (T a) noexcept
    {
        const auto d = ipow<4> (a);
        return ipow<3> (d);
    }

    template <int exp, typename T>
    constexpr std::enable_if_t<exp == 13, T>
        ipow (T a) noexcept
    {
        return ipow<12> (a) * a;
    }

    template <int exp, typename T>
    constexpr std::enable_if_t<exp == 14, T>
        ipow (T a) noexcept
    {
        const auto g = ipow<7> (a);
        return ipow<2> (g);
    }

    template <int exp, typename T>
    constexpr std::enable_if_t<exp == 15, T>
        ipow (T a) noexcept
    {
        const auto e = ipow<5> (a);
        return ipow<3> (e);
    }

    template <int exp, typename T>
    constexpr std::enable_if_t<exp == 16, T>
        ipow (T a) noexcept
    {
        const auto h = ipow<8> (a);
        return ipow<2> (h);
    }

    // For exponents larger than 16, use exponantiation by squaring (https://en.wikipedia.org/wiki/Exponentiation_by_squaring)

    template <int exp, typename T>
    constexpr std::enable_if_t<(exp > 16) && exp % 2 == 0, T>
        ipow (T a) noexcept
    {
        return ipow<exp / 2> (ipow<2> (a));
    }

    template <int exp, typename T>
    constexpr std::enable_if_t<(exp > 16) && exp % 2 != 0, T>
        ipow (T a) noexcept
    {
        return ipow<(exp - 1) / 2> (ipow<2> (a)) * a;
    }
} // namespace Power
} // namespace chowdsp
