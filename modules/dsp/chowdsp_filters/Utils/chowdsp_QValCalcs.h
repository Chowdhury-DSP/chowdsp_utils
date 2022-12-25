#pragma once

namespace chowdsp
{
/** Useful methods for computing Q values for filters */
namespace QValCalcs
{
    /** Computes Q values for a Butterworth filter of order N */
    template <typename T, size_t N>
    constexpr std::array<T, N / 2> butterworth_Qs()
    {
        std::array<T, N / 2> qVals {};

        size_t k = 1;
        const auto lim = N / 2;

        while (k <= lim)
        {
            auto b = static_cast<T> (-2) * gcem::cos ((T (2 * k) + N - 1) * juce::MathConstants<T>::pi / ((T) 2 * N));
            qVals[k - 1] = static_cast<T> (1) / b;
            k += 1;
        }

        return qVals;
    }
} // namespace QValCalcs
} // namespace chowdsp
