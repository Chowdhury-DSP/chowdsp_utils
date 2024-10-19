#pragma once

namespace chowdsp
{
/** Extensions of juce::FloatVectorOperations */
namespace FloatVectorOperations
{
#if JUCE_MAC
    /** Returns true if FloatVectorOperations will be performed using the Apple vDSP framework */
    [[maybe_unused]] bool isUsingVDSP();
#endif

    /** Divides a scalar value by the src vector. */
    template <typename T>
    std::enable_if_t<std::is_floating_point_v<T>, void> divide (T* dest, T dividend, const T* divisor, int numValues) noexcept;

    /** Divides two vectors. */
    template <typename T>
    std::enable_if_t<std::is_floating_point_v<T>, void> divide (T* dest, const T* dividend, const T* divisor, int numValues) noexcept;

    /** Sums all the values in the given array. */
    template <typename T>
    std::enable_if_t<std::is_floating_point_v<T>, T> accumulate (const T* src, int numValues) noexcept;

    /** Computes the inner product between the two arrays. */
    template <typename T>
    std::enable_if_t<std::is_floating_point_v<T>, T> innerProduct (const T* src1, const T* src2, int numValues) noexcept;

    /** Finds the absolute maximum value in the given array. */
    template <typename T>
    std::enable_if_t<std::is_floating_point_v<T>, T> findAbsoluteMaximum (const T* src, int numValues) noexcept;

    /** Takes the exponent of each value to an integer power. */
    template <typename T>
    std::enable_if_t<std::is_floating_point_v<T>, void> integerPower (T* dest, const T* src, int exponent, int numValues) noexcept;

    /** Computes the Root-Mean-Square average of the input data. */
    template <typename T>
    std::enable_if_t<std::is_floating_point_v<T>, T> computeRMS (const T* src, int numValues) noexcept;

    /** Counts the number of Inf and NaN values in the input data */
    template <typename T>
    std::enable_if_t<std::is_floating_point_v<T>, int> countInfsAndNaNs (const T* src, int numValues) noexcept;

    /**
     * Equivalent implementation to std::rotate, but without allocating memory.
     *
     * scratchData must point to at least numToRotate values.
     */
    template <typename T>
    std::enable_if_t<std::is_floating_point_v<T>, void> rotate (T* data, int numToRotate, int totalNumValues, T* scratchData) noexcept;

} // namespace FloatVectorOperations
} // namespace chowdsp
