#pragma once

namespace chowdsp
{
/** Anti-derivatives of the tanh function */
namespace TanhIntegrals
{
    /** First anti-derivative of tanh */
    template <typename T>
    constexpr T tanhAD1 (T x) noexcept
    {
        return gcem::log (gcem::cosh (x));
    }

    /** Second anti-derivative of tanh */
    template <typename T>
    constexpr T tanhAD2 (T x) noexcept
    {
        using Polylogarithm::Li2, Power::ipow;
        const auto expVal = gcem::exp ((T) -2 * x);
        return (T) 0.5 * (Li2 (-expVal) - x * (x + (T) 2 * gcem::log (expVal + (T) 1) - (T) 2 * gcem::log (gcem::cosh (x)))) + (ipow<2> (juce::MathConstants<T>::pi) / (T) 24);
    }
} // namespace TanhIntegrals
} // namespace chowdsp
