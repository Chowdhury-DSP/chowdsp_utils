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
} // namespace chowdsp
