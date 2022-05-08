#pragma once

namespace chowdsp
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
} // namespace chowdsp
