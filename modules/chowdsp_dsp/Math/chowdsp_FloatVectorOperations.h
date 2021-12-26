#pragma once

/** Extensions of juce::FloatVectorOperations */
namespace chowdsp::FloatVectorOperations
{
/** Sums all the values in the given array. */
[[maybe_unused]] float accumulate (const float* src, int numValues) noexcept;

/** Sums all the values in the given array. */
[[maybe_unused]] double accumulate (const double* src, int numValues) noexcept;

/** Computes the inner product between the two arrays. */
[[maybe_unused]] float innerProduct (const float* src1, const float* src2, int numValues) noexcept;

/** Computes the inner product between the two arrays. */
[[maybe_unused]] double innerProduct (const double* src1, const double* src2, int numValues) noexcept;
} // namespace chowdsp::FloatVectorOperations
