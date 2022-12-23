#pragma once

namespace chowdsp
{
/** Helper methods for implementing Vicanek=style biquad filters (https://www.vicanek.de/articles/BiquadFits.pdf) */
namespace VicanekHelpers
{
    /** Clamps a Q-value into the range where Vicanek's method is numerically robust */
    template <typename T>
    [[maybe_unused]] inline T clampQVicanek (T qVal)
    {
        using NumericType = SampleTypeHelpers::NumericType<T>;
        CHOWDSP_USING_XSIMD_STD (max);
        CHOWDSP_USING_XSIMD_STD (min);

        static constexpr auto lowerBound = NumericType (std::is_same_v<NumericType, float> ? 0.55 : 0.5);
        static constexpr auto upperBound = NumericType (std::is_same_v<NumericType, float> ? 4.94 : 27.0);

        // Vicanek method has numerical problems for Q-values outside of this range
        jassert (SIMDUtils::all (qVal >= (T) lowerBound));
        jassert (SIMDUtils::all (qVal < (T) upperBound));
        return min ((T) upperBound, max ((T) lowerBound, qVal));
    }

    /** Computes denominator coefficients using Vicanek's method. Using w0 = 2 * pi * fc / fs */
    template <typename T>
    [[maybe_unused]] inline void computeVicanekDenominator (T w0, T qVal, T (&a)[3])
    {
        CHOWDSP_USING_XSIMD_STD (sqrt);
        CHOWDSP_USING_XSIMD_STD (exp);
        CHOWDSP_USING_XSIMD_STD (cos);
        CHOWDSP_USING_XSIMD_STD (cosh);
        using Power::ipow;

        const auto inv2Q = (T) 0.5 / qVal;

        const auto expmqw = exp (-inv2Q * w0);
        const auto cos_arg = sqrt ((T) 1 - ipow<2> (inv2Q)) * w0;
        a[0] = (T) 1;
        a[1] = -(T) 2 * expmqw * SIMDUtils::select (inv2Q <= (T) 1, cos (cos_arg), cosh (-cos_arg));
        a[2] = ipow<2> (expmqw);
    }

    /** Computes the phi values used by Vicanek's method. Using w0 = 2 * pi * fc / fs */
    template <typename T>
    [[maybe_unused]] inline auto computeVicanekPhiVals (T w0)
    {
        CHOWDSP_USING_XSIMD_STD (sin);
        using Power::ipow;

        const auto sinpd2 = sin ((T) 0.5 * w0);
        const auto p1 = ipow<2> (sinpd2);
        const auto p0 = (T) 1 - p1;
        const auto p2 = (T) 4 * p0 * p1;

        return std::make_tuple (p0, p1, p2);
    }

    /** Computes the "A" values used by Vicanek's method. */
    template <typename T>
    [[maybe_unused]] inline auto computeVicanekAVals (const T (&a)[3])
    {
        using Power::ipow;
        const auto A0 = ipow<2> ((T) 1 + a[1] + a[2]);
        const auto A1 = ipow<2> ((T) 1 - a[1] + a[2]);
        const auto A2 = (T) -4 * a[2];

        return std::make_tuple (A0, A1, A2);
    }

    /** Computes pole info using Vicanek's method */
    template <typename T, typename NumericType>
    [[maybe_unused]] inline auto computeVicanekPolesAngular (T wc, T qVal, NumericType fs, T (&a)[3])
    {
        const auto w0 = wc / fs;
        computeVicanekDenominator (w0, qVal, a);

        const auto [p0, p1, p2] = computeVicanekPhiVals (w0);
        const auto [A0, A1, A2] = computeVicanekAVals (a);

        return std::make_tuple (p0, p1, p2, A0, A1, A2);
    }

    /** Computes biquad numerator coefficients from Vicanek's B0, B1, B2. Some filters may skip this if they have a more efficit way to compute the coefficients. */
    template <typename T>
    [[maybe_unused]] inline void computeVicanekBiquadNumerator (T B0, T B1, T B2, T (&b)[3])
    {
        CHOWDSP_USING_XSIMD_STD (sqrt);

        const auto sqrtB0 = sqrt (B0);
        const auto sqrtB1 = sqrt (B1);
        const auto W = (T) 0.5 * (sqrtB0 + sqrtB1);

        b[0] = (T) 0.5 * (W + sqrt (W * W + B2));
        b[1] = (T) 0.5 * (sqrtB0 - sqrtB1);
        b[2] = -B2 / ((T) 4 * b[0]);
    }
} // namespace VicanekHelpers
} // namespace chowdsp
