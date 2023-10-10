#pragma once

namespace chowdsp
{
/**
 * Approximation functions for trigonometric functions.
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

        template <typename T>
        xsimd::batch<T> truncate (xsimd::batch<T> x)
        {
            return xsimd::to_float (xsimd::to_int (x));
        }

        /** Fast method to wrap a value into the range [-pi, pi] */
        template <typename T>
        T fast_mod_mpi_pi (T x)
        {
            using NumericType = SampleTypeHelpers::NumericType<T>;
            x += juce::MathConstants<NumericType>::pi;
            const auto mod = x - juce::MathConstants<NumericType>::twoPi * truncate (x * recip_2pi<NumericType>);
            return SIMDUtils::select (x >= (T) 0, mod, mod + juce::MathConstants<NumericType>::twoPi) - juce::MathConstants<NumericType>::pi;
        }

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

    template <int order = 7, typename T = float>
    T sin_3angle (T x)
    {
        return sin_3angle_mpi_pi<order, T> (detail::fast_mod_mpi_pi (x));
    }

    // @TODO:
    // - triple-angle approximations
    // - better docs
    // - cosine
    // - sine/cosine together
    // - tests
} // namespace TrigApprox
} // namespace chowdsp
