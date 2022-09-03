#pragma once

namespace chowdsp
{
/** Anti-derivatives of the tanh function */
namespace TanhIntegrals
{
    /** First anti-derivative of tanh */
    template <typename T>
    inline T tanhAD1 (T x) noexcept
    {
        // It might be possible to optimize this with something like
        // return std::log ((T) 0.5) + x + std::log ((T) 1 + std::exp ((T) -2 * x));
        // but I haven't been able to measure a performance gain, so we'll
        // stick with the more readable version:

        return std::log (std::cosh (x));
    }

    /** Second anti-derivative of tanh */
    template <typename T>
    inline T tanhAD2 (T x) noexcept
    {
        using Polylogarithm::Li2, Power::ipow;
        const auto expVal = std::exp ((T) -2 * x);
        return (T) 0.5 * (Li2 (-expVal) - x * (x + (T) 2 * std::log (expVal + (T) 1) - (T) 2 * std::log (std::cosh (x)))) + (ipow<2> (juce::MathConstants<T>::pi) / (T) 24);
    }
} // namespace TanhIntegrals
} // namespace chowdsp
