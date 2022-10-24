#pragma once

JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wsign-conversion")

namespace chowdsp
{
/** Transforming filters with some linear transformations */
namespace LinearTransforms
{
    /** Transforms a filter with some amount of feedback. */
    template <int Order, typename T, bool normalize = true>
    inline void transformFeedback (T (&b_coeffs)[Order + 1], T (&a_coeffs)[Order + 1], T G)
    {
        const auto a0 = a_coeffs[0] - G * b_coeffs[0];
        const auto a0_inv = normalize ? (T) 1 / a0 : (T) 1;

        a_coeffs[0] = normalize ? (T) 1 : a0;
        for (int i = 1; i <= Order; ++i)
            a_coeffs[i] = (a_coeffs[i] - G * b_coeffs[i]) * a0_inv;

        for (int i = 0; i <= Order; ++i)
            b_coeffs[i] = b_coeffs[i] * a0_inv;
    }
} // namespace LinearTransforms
} // namespace chowdsp

JUCE_END_IGNORE_WARNINGS_GCC_LIKE
