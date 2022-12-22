#pragma once

namespace chowdsp::SIMDUtils
{
/** Returns true if the pointer is aligned to its required SIMD byte boundary. */
template <typename T>
static bool isAligned (const T* p) noexcept
{
    static constexpr auto RegisterSize = sizeof (xsimd::batch<T>);
    uintptr_t bitmask = RegisterSize - 1;
    return ((uintptr_t) p & bitmask) == 0;
}

/**
 * A handy function to round up a pointer to the nearest multiple of a given number of bytes.
 * alignmentBytes must be a power of two.
 */
template <typename Type, typename IntegerType>
inline Type* snapPointerToAlignment (Type* basePointer, IntegerType alignmentBytes) noexcept
{
    return (Type*) ((((size_t) basePointer) + (alignmentBytes - 1)) & ~(alignmentBytes - 1));
}

/** Returns the next aligned pointer after this one. */
template <typename T>
static T* getNextAlignedPtr (T* p) noexcept
{
    static constexpr auto RegisterSize = sizeof (xsimd::batch<std::remove_const_t<T>>);
    return snapPointerToAlignment (p, RegisterSize);
}
} // namespace chowdsp::SIMDUtils
