#pragma once

namespace chowdsp
{
/** A handful of extra maths functions */
namespace Math
{
    /**
 * log2 for integer values.
 *
 * For numbers that are not a power of two, this method will round up.
 */
    template <typename IntType>
    inline int log2 (IntType n)
    {
        jassert (n > 0); // Log2 is undefined for numbers less than or equal to zero!"
        return ((n <= 1) ? 0 : 2 + (int) std::log2 ((n - 1) / 2));
    }

    /**
 * Divides two numbers and rounds up if there is a remainder.
 *
 * This is often useful for figuring out haw many SIMD registers are needed
 * to contain a given number of scalar values.
 */
    template <typename T>
    constexpr T ceiling_divide (T num, T den)
    {
        return (num + den - 1) / den;
    }

    /**
 * Returns 1 if the input is positive, -1 if the input is negative,
 * and 0 if the input is zero.
 */
    template <typename T>
    inline T sign (T val)
    {
        return (T (0) < val) - (val < T (0));
    }

    /**
 * Returns 1 if the input is positive, -1 if the input is negative,
 * and 0 if the input is zero.
 */
    template <typename T>
    inline xsimd::batch<T> sign (xsimd::batch<T> val)
    {
        using v_type = xsimd::batch<T>;
        const auto positive = xsimd::select (val > v_type ((T) 0), v_type ((T) 1), v_type ((T) 0));
        const auto negative = xsimd::select (val < v_type ((T) 0), v_type ((T) 1), v_type ((T) 0));
        return positive - negative;
    }
} // namespace Math
} // namespace chowdsp
