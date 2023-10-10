#pragma once

namespace chowdsp
{
/**
 * Approximation functions for trigonometric functions.
 */
namespace TrigApprox
{
    namespace detail
    {
        template <typename T>
        static constexpr auto recip_2pi = (T) 1 / juce::MathConstants<T>::twoPi;

        /** Fast method to wrap a value into the range [-pi, pi] */
        template <typename T>
        T fast_mod_mpi_pi (T x)
        {
            x += juce::MathConstants<T>::pi;
            const auto mod = x - juce::MathConstants<T>::twoPi * T ((int) (x * recip_2pi<T>) );
            return (x >= (T) 0 ? mod : mod + juce::MathConstants<T>::twoPi) - juce::MathConstants<T>::pi;
        }
    }

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
        static constexpr auto pi = juce::MathConstants<T>::pi;
        jassert (SIMDUtils::all (x <= pi)
                 && SIMDUtils::all (x >= -pi));

        CHOWDSP_USING_XSIMD_STD (abs);
        const auto abs_x = abs (x);
        const auto x_pi_minus_x = abs_x * (pi - abs_x);
        const auto numerator = (T) 16 * x_pi_minus_x;

        static constexpr auto five_pi_sq = (T) 5 * pi * pi;
        const auto denominator = five_pi_sq - (T) 4 * x_pi_minus_x;

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
        static constexpr auto pi = juce::MathConstants<T>::pi;
        jassert (SIMDUtils::all (x <= pi)
                 && SIMDUtils::all (x >= -pi));

        CHOWDSP_USING_XSIMD_STD (abs);
        static constexpr auto four_over_pi_sq = (T) 4 / (pi * pi);
        const auto abs_x = abs (x);

        return four_over_pi_sq * x * (pi - abs_x);
    }

    template <typename T>
    T sin_1st_order (T x)
    {
        return sin_1st_order_mpi_pi (detail::fast_mod_mpi_pi (x));
    }

    // @TODO:
    // - triple-angle approximations
    // - better docs
    // - cosine
    // - sine/cosine together
    // - tests
} // namespace TrigApprox
} // namespace chowdsp
