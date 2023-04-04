#pragma once

namespace chowdsp
{
/** Extensions of juce::FloatVectorOperations */
namespace FloatVectorOperations
{
    /** Returns true if FloatVectorOperations will be performed using the Apple vDSP framework */
    [[maybe_unused]] bool isUsingVDSP();

    /** Divides a scalar value by the src vector. */
    [[maybe_unused]] void divide (float* dest, const float* dividend, const float* divisor, int numValues) noexcept;

    /** Divides a scalar value by the src vector. */
    [[maybe_unused]] void divide (double* dest, const double* dividend, const double* divisor, int numValues) noexcept;

    /** Divides a scalar value by the src vector. */
    [[maybe_unused]] void divide (float* dest, float dividend, const float* divisor, int numValues) noexcept;

    /** Divides a scalar value by the src vector. */
    [[maybe_unused]] void divide (double* dest, double dividend, const double* divisor, int numValues) noexcept;

    /** Sums all the values in the given array. */
    [[maybe_unused]] float accumulate (const float* src, int numValues) noexcept;

    /** Sums all the values in the given array. */
    [[maybe_unused]] double accumulate (const double* src, int numValues) noexcept;

    /** Computes the inner product between the two arrays. */
    [[maybe_unused]] float innerProduct (const float* src1, const float* src2, int numValues) noexcept;

    /** Computes the inner product between the two arrays. */
    [[maybe_unused]] double innerProduct (const double* src1, const double* src2, int numValues) noexcept;

    /** Finds the absolute maximum value in the given array. */
    [[maybe_unused]] float findAbsoluteMaximum (const float* src, int numValues) noexcept;

    /** Finds the absolute maximum value in the given array. */
    [[maybe_unused]] double findAbsoluteMaximum (const double* src, int numValues) noexcept;

    /** Takes the exponent of each value to an integer power. */
    [[maybe_unused]] void integerPower (float* dest, const float* src, int exponent, int numValues) noexcept;

    /** Takes the exponent of each value to an integer power. */
    [[maybe_unused]] void integerPower (double* dest, const double* src, int exponent, int numValues) noexcept;

    /** Computes the Root-Mean-Square average of the input data. */
    [[maybe_unused]] float computeRMS (const float* src, int numValues) noexcept;

    /** Computes the Root-Mean-Square average of the input data. */
    [[maybe_unused]] double computeRMS (const double* src, int numValues) noexcept;

    /** Counts the number of NaN values in the input data */
    [[maybe_unused]] int countNaNs (const float* src, int numValues) noexcept;

    /** Counts the number of NaN values in the input data */
    [[maybe_unused]] int countNaNs (const double* src, int numValues) noexcept;

    /** Counts the number of Inf values in the input data */
    [[maybe_unused]] int countInfs (const float* src, int numValues) noexcept;

    /** Counts the number of Inf values in the input data */
    [[maybe_unused]] int countInfs (const double* src, int numValues) noexcept;

    /**
     * Equivalent implementation to std::rotate, but without allocating memory.
     *
     * scratchData must point to at least numToRotate values.
     */
    [[maybe_unused]] void rotate (float* data, int numToRotate, int totalNumValues, float* scratchData) noexcept;

    /**
     * Equivalent implementation to std::rotate, but without allocating memory.
     *
     * scratchData must point to at least numToRotate values.
     */
    [[maybe_unused]] void rotate (double* data, int numToRotate, int totalNumValues, double* scratchData) noexcept;

} // namespace FloatVectorOperations
} // namespace chowdsp
