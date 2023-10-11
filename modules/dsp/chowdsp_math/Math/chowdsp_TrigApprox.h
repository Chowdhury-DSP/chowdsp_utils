#pragma once

namespace chowdsp
{
/**
 * Approximation functions for trigonometric functions.
 *
 * References:
 * - Sine plots: https://www.desmos.com/calculator/rnsmcx6wb5
 * - Cosine plots: https://www.desmos.com/calculator/tgt3dejrgr
 */
namespace TrigApprox
{
#ifndef DOXYGEN
    namespace detail
    {
        template <typename T>
        static constexpr auto recip_2pi = (T) 1 / juce::MathConstants<T>::twoPi;

        template <typename T>
        T truncate (T x)
        {
            return T ((int) x);
        }

#if ! CHOWDSP_NO_XSIMD
        template <typename T>
        xsimd::batch<T> truncate (xsimd::batch<T> x)
        {
            return xsimd::to_float (xsimd::to_int (x));
        }
#endif

        /** Fast method to wrap a value into the range [-pi, pi] */
        template <typename T>
        T fast_mod_mpi_pi (T x)
        {
            using NumericType = SampleTypeHelpers::NumericType<T>;
            x += juce::MathConstants<NumericType>::pi;
            const auto mod = x - juce::MathConstants<NumericType>::twoPi * truncate (x * recip_2pi<NumericType>);
            return SIMDUtils::select (x >= (T) 0, mod, mod + juce::MathConstants<NumericType>::twoPi) - juce::MathConstants<NumericType>::pi;
        }

        /** Shifts range [-pi, pi] to [-pi/2, 3pi/2] for cosine calculations */
        template <typename T>
        T shift_cosine_range (T x)
        {
            using NumericType = SampleTypeHelpers::NumericType<T>;
            return SIMDUtils::select (x >= -juce::MathConstants<NumericType>::halfPi,
                                      x,
                                      x + juce::MathConstants<NumericType>::twoPi);
        }

        /**
         * Polynomial approximations of sine for [-pi/3, pi]
         *
         * Starts as a Taylor approximation, but then tries to fix the end-points,
         * minimize max error.
         *
         * Reference: https://www.wolframcloud.com/env/chowdsp/sin_approx.nb
         */
        template <typename T, int order>
        T sin_o3 (T theta, T theta_sq, [[maybe_unused]] T theta_quad)
        {
            using NumericType = SampleTypeHelpers::NumericType<T>;
            static_assert (order % 2 == 1 && order > 1 && order <= 9, "Order must be an odd integer within [3,9]");

            if constexpr (order == 9)
            {
                const auto s1 = NumericType (1) + NumericType (-1.66666662492e-1) * theta_sq;
                const auto s2 = NumericType (8.33330303755e-3) + NumericType (-1.98339763113e-4) * theta_sq;
                const auto s3 = NumericType (2.68396896919e-6) * theta_quad;
                return theta * (s1 + (s2 + s3) * theta_quad);
            }
            else if constexpr (order == 7)
            {
                const auto s1 = NumericType (1) + NumericType (-1.66664550673e-1) * theta_sq;
                const auto s2 = NumericType (8.32622561667e-3) + NumericType (-1.90698699142e-4) * theta_sq;
                return theta * (s1 + s2 * theta_quad);
            }
            else if constexpr (order == 5)
            {
                const auto s1 = NumericType (1) + NumericType (-1.66531874421e-1) * theta_sq;
                const auto s2 = NumericType (7.99611485830e-3) * theta_quad;
                return theta * (s1 + s2);
            }
            else if constexpr (order == 3)
            {
                const auto s1 = NumericType (1) + NumericType (-1.57763153266e-1) * theta_sq;
                return theta * s1;
            }
        }

        /**
         * Polynomial approximations of cosine for [-pi/3, pi]
         *
         * Starts as a Taylor approximation, but then tries to fix the end-points,
         * minimize max error.
         *
         * Reference: https://www.wolframcloud.com/env/chowdsp/cos_approx.nb
         */
        template <typename T, int order>
        T cos_o3 ([[maybe_unused]] T theta, T theta_sq, [[maybe_unused]] T theta_quad)
        {
            using NumericType = SampleTypeHelpers::NumericType<T>;
            static_assert (order % 2 == 0 && order > 2 && order <= 8, "Order must be an odd integer within [2,8]");

            if constexpr (order == 8)
            {
                const auto c1 = (NumericType) -0.5 + (NumericType) 4.16666094252e-2 * theta_sq;
                const auto c2 = (NumericType) -1.38854590421e-3 + (NumericType) 2.42367173361e-5 * theta_sq;
                return (NumericType) 1 + theta_sq * (c1 + c2 * theta_quad);
            }
            else if constexpr (order == 6)
            {
                const auto c1 = (NumericType) -0.5 + (NumericType) 4.16527333677e-2 * theta_sq;
                const auto c2 = (NumericType) -1.34931392239e-3 * theta_quad;
                return (NumericType) 1 + theta_sq * (c1 + c2);
            }
            else if constexpr (order == 4)
            {
                const auto c1 = (NumericType) -0.5 + (NumericType) 4.01730450758e-2 * theta_sq;
                return (NumericType) 1 + theta_sq * c1;
            }
        }
    } // namespace detail
#endif // DOXYGEN

    /**
     * Bhaskara I's sine approximation valid for x in [-pi, pi]
     *
     * Max error: 1.64e-3
     *
     * Reference: https://en.wikipedia.org/wiki/Bh%C4%81skara_I%27s_sine_approximation_formula
     */
    template <typename T>
    T sin_bhaskara_mpi_pi (T x)
    {
        using NumericType = SampleTypeHelpers::NumericType<T>;
        static constexpr auto pi = juce::MathConstants<NumericType>::pi;
        jassert (SIMDUtils::all (x <= pi)
                 && SIMDUtils::all (x >= -pi));

        CHOWDSP_USING_XSIMD_STD (abs);
        const auto abs_x = abs (x);
        const auto x_pi_minus_x = abs_x * (pi - abs_x);
        const auto numerator = (NumericType) 16 * x_pi_minus_x;

        static constexpr auto five_pi_sq = (NumericType) 5 * pi * pi;
        const auto denominator = five_pi_sq - (NumericType) 4 * x_pi_minus_x;

        const auto result = numerator / denominator;
        return SIMDUtils::select (x >= 0, result, -result);
    }

    /** Bhaskara I's sine approximation valid across the whole range. */
    template <typename T>
    T sin_bhaskara (T x)
    {
        return sin_bhaskara_mpi_pi (detail::fast_mod_mpi_pi (x));
    }

    /**
     * First-order sine approximation.
     *
     * Max error: 5.6e-2
     */
    template <typename T>
    T sin_1st_order_mpi_pi (T x)
    {
        using NumericType = SampleTypeHelpers::NumericType<T>;
        static constexpr auto pi = juce::MathConstants<NumericType>::pi;
        jassert (SIMDUtils::all (x <= pi)
                 && SIMDUtils::all (x >= -pi));

        CHOWDSP_USING_XSIMD_STD (abs);
        static constexpr auto four_over_pi_sq = (NumericType) 4 / (pi * pi);
        const auto abs_x = abs (x);

        return four_over_pi_sq * x * (pi - abs_x);
    }

    /** Full-range first-order sine approximation. */
    template <typename T>
    T sin_1st_order (T x)
    {
        return sin_1st_order_mpi_pi (detail::fast_mod_mpi_pi (x));
    }

    /**
     * Sine approximation using a Taylor approximation on the
     * range [-pi/3, pi/3], expanded to [-pi, pi] using the
     * triple angle formula.
     *
     * The approximation is parameterized on the order of
     * the Taylor approximation, which _must_ be an odd integer.
     *
     * Max error (9th-order): 3.5e-10
     * Max error (7th-order): 1.142e-7
     * Max error (5th-order): 4.3e-5
     * Max error (3th-order): 7.3e-3
     */
    template <int order = 7, typename T = float>
    T sin_3angle_mpi_pi (T x)
    {
        using NumericType = SampleTypeHelpers::NumericType<T>;
        [[maybe_unused]] static constexpr auto pi = juce::MathConstants<NumericType>::pi;
        jassert (SIMDUtils::all (x <= pi)
                 && SIMDUtils::all (x >= -pi));

        const auto theta_o3 = x * NumericType (1.0 / 3.0);
        const auto theta_o3_sq = theta_o3 * theta_o3;
        const auto theta_o3_quad = theta_o3_sq * theta_o3_sq;
        const auto sin_o3 = detail::sin_o3<T, order> (theta_o3, theta_o3_sq, theta_o3_quad);
        return sin_o3 * ((NumericType) 3 + (NumericType) -4 * sin_o3 * sin_o3);
    }

    /** Full-range triple-angle sine approximation. */
    template <int order = 7, typename T = float>
    T sin_3angle (T x)
    {
        return sin_3angle_mpi_pi<order, T> (detail::fast_mod_mpi_pi (x));
    }

    /**
     * Bhaskara I's cosine approximation valid for x in [-pi, pi]
     *
     * Max error: 1.64e-3
     *
     * Reference: https://en.wikipedia.org/wiki/Bh%C4%81skara_I%27s_sine_approximation_formula
     */
    template <typename T>
    T cos_bhaskara_mpi_pi (T x)
    {
        using NumericType = SampleTypeHelpers::NumericType<T>;
        static constexpr auto pi = juce::MathConstants<NumericType>::pi;
        jassert (SIMDUtils::all (x <= pi)
                 && SIMDUtils::all (x >= -pi));

        x = detail::shift_cosine_range (x);
        const auto needs_flip = x > juce::MathConstants<NumericType>::halfPi;

        x = SIMDUtils::select (needs_flip, x - pi, x);

        static constexpr auto pi_sq = pi * pi;
        const auto x_sq = x * x;
        const auto result = (pi_sq - (NumericType) 4 * x_sq) / (pi_sq + x_sq);
        return SIMDUtils::select (needs_flip, -result, result);
    }

    /** Bhaskara I's cosine approximation valid across the whole range. */
    template <typename T>
    T cos_bhaskara (T x)
    {
        return cos_bhaskara_mpi_pi (detail::fast_mod_mpi_pi (x));
    }

    /**
     * First-order cosine approximation.
     *
     * Max error: 5.6e-2
     */
    template <typename T>
    T cos_1st_order_mpi_pi (T x)
    {
        using NumericType = SampleTypeHelpers::NumericType<T>;
        [[maybe_unused]] static constexpr auto pi = juce::MathConstants<NumericType>::pi;
        jassert (SIMDUtils::all (x <= pi)
                 && SIMDUtils::all (x >= -pi));

        x = detail::shift_cosine_range (x);
        return -sin_1st_order_mpi_pi (x - juce::MathConstants<NumericType>::halfPi);
    }

    /** Full-range first-order cosine approximation. */
    template <typename T>
    T cos_1st_order (T x)
    {
        return cos_1st_order_mpi_pi (detail::fast_mod_mpi_pi (x));
    }

    /**
     * Cosine approximation using a Taylor approximation on the
     * range [-pi/3, pi/3], expanded to [-pi, pi] using the
     * triple angle formula.
     *
     * The approximation is parameterized on the order of
     * the Taylor approximation, which _must_ be an odd integer.
     *
     * Max error (8th-order): 7.25e-9
     * Max error (6th-order): 2.24e-6
     * Max error (4th-order): 7.93e-4
     */
    template <int order = 6, typename T = float>
    T cos_3angle_mpi_pi (T x)
    {
        using NumericType = SampleTypeHelpers::NumericType<T>;
        [[maybe_unused]] static constexpr auto pi = juce::MathConstants<NumericType>::pi;
        jassert (SIMDUtils::all (x <= pi)
                 && SIMDUtils::all (x >= -pi));

        const auto theta_o3 = x * NumericType (1.0 / 3.0);
        const auto theta_o3_sq = theta_o3 * theta_o3;
        const auto theta_o3_quad = theta_o3_sq * theta_o3_sq;
        const auto cos_o3 = detail::cos_o3<T, order> (theta_o3, theta_o3_sq, theta_o3_quad);
        return cos_o3 * ((NumericType) -3 + (NumericType) 4 * cos_o3 * cos_o3);
    }

    /** Full-range triple-angle cosine approximation. */
    template <int order = 6, typename T = float>
    T cos_3angle (T x)
    {
        return cos_3angle_mpi_pi<order, T> (detail::fast_mod_mpi_pi (x));
    }

    /**
     * Combined sine/cosine approximation, using the triple-angle
     * approximations described above.
     */
    template <int sin_order = 7, int cos_order = 6, typename T = float>
    auto sin_cos_3angle_mpi_pi (T x)
    {
        using NumericType = SampleTypeHelpers::NumericType<T>;
        [[maybe_unused]] static constexpr auto pi = juce::MathConstants<NumericType>::pi;
        jassert (SIMDUtils::all (x <= pi)
                 && SIMDUtils::all (x >= -pi));

        const auto theta_o3 = x * NumericType (1.0 / 3.0);
        const auto theta_o3_sq = theta_o3 * theta_o3;
        const auto theta_o3_quad = theta_o3_sq * theta_o3_sq;

        const auto sin_o3 = detail::sin_o3<T, sin_order> (theta_o3, theta_o3_sq, theta_o3_quad);
        const auto s = sin_o3 * ((NumericType) 3 + (NumericType) -4 * sin_o3 * sin_o3);

        const auto cos_o3 = detail::cos_o3<T, cos_order> (theta_o3, theta_o3_sq, theta_o3_quad);
        const auto c = cos_o3 * ((NumericType) -3 + (NumericType) 4 * cos_o3 * cos_o3);

        return std::make_tuple (s, c);
    }

    /** Full-range triple-angle sine/cosine approximation. */
    template <int sin_order = 7, int cos_order = 6, typename T = float>
    auto sin_cos_3angle (T x)
    {
        return sin_cos_3angle_mpi_pi<sin_order, cos_order, T> (detail::fast_mod_mpi_pi (x));
    }
} // namespace TrigApprox
} // namespace chowdsp
