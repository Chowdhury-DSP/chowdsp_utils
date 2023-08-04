#pragma once

namespace chowdsp
{
/**
 * Implementations of polylogarithm functions.
 *
 * Based on the implementations found at: https://github.com/Expander/polylogarithm
 */
namespace Polylogarithm
{
    /** real polylogarithm with n=2 (dilogarithm). */
    template <typename T>
    inline T Li2 (T x) noexcept
    {
        constexpr auto PI_ = juce::MathConstants<T>::pi;
        constexpr T P[] = {
            (T) 0.9999999999999999502e+0,
            (T) -2.6883926818565423430e+0,
            (T) 2.6477222699473109692e+0,
            (T) -1.1538559607887416355e+0,
            (T) 2.0886077795020607837e-1,
            (T) -1.0859777134152463084e-2
        };
        constexpr T Q[] = {
            (T) 1.0000000000000000000e+0,
            (T) -2.9383926818565635485e+0,
            (T) 3.2712093293018635389e+0,
            (T) -1.7076702173954289421e+0,
            (T) 4.1596017228400603836e-1,
            (T) -3.9801343754084482956e-2,
            (T) 8.2743668974466659035e-4
        };

        T y = 0, r = 0, s = 1;

        // transform to [0, 1/2]
        if (x < (T) -1)
        {
            const auto l = std::log ((T) 1 - x);
            y = (T) 1 / ((T) 1 - x);
            r = -PI_ * PI_ / (T) 6 + l * ((T) 0.5 * l - std::log (-x));
            s = (T) 1;
        }
        else if (juce::exactlyEqual (x, (T) -1))
        {
            return -PI_ * PI_ / (T) 12;
        }
        else if (x < (T) 0)
        {
            const auto l = std::log1p (-x);
            y = x / (x - (T) 1);
            r = (T) -0.5 * l * l;
            s = (T) -1;
        }
        else if (juce::exactlyEqual (x, (T) 0))
        {
            return (T) 0;
        }
        else if (x < (T) 0.5)
        {
            y = x;
            r = (T) 0;
            s = (T) 1;
        }
        else if (x < (T) 1)
        {
            y = (T) 1 - x;
            r = PI_ * PI_ / (T) 6 - std::log (x) * std::log (y);
            s = (T) -1;
        }
        else if (juce::exactlyEqual (x, (T) 1))
        {
            return PI_ * PI_ / (T) 6;
        }
        else if (x < (T) 2)
        {
            const auto l = std::log (x);
            y = (T) 1 - (T) 1 / x;
            r = PI_ * PI_ / (T) 6 - l * (std::log (y) + (T) 0.5 * l);
            s = (T) 1;
        }
        else
        {
            const auto l = std::log (x);
            y = (T) 1 / x;
            r = PI_ * PI_ / (T) 3 - (T) 0.5 * l * l;
            s = (T) -1;
        }

        const auto y2 = y * y;
        const auto y4 = y2 * y2;
        const auto p = P[0] + y * P[1] + y2 * (P[2] + y * P[3]) + y4 * (P[4] + y * P[5]);
        const auto q = Q[0] + y * Q[1] + y2 * (Q[2] + y * Q[3]) + y4 * (Q[4] + y * Q[5] + y2 * Q[6]);

        return r + s * y * p / q;
    }
} // namespace Polylogarithm
} // namespace chowdsp
