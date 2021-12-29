#pragma once

namespace chowdsp::SIMDUtils
{
/**
 * Fast implementation of the JUCE
 * FastMathApproximations::sin function
 * with SIMD optimisation.
 */
template <typename T>
inline juce::dsp::SIMDRegister<T> fastsinSIMD (juce::dsp::SIMDRegister<T> x) noexcept
{
    const auto m11511339840 = juce::dsp::SIMDRegister<T> ((T) 11511339840);
    const auto m1640635920 = juce::dsp::SIMDRegister<T> ((T) 1640635920);
    const auto m52785432 = juce::dsp::SIMDRegister<T> ((T) 52785432);
    const auto m479249 = juce::dsp::SIMDRegister<T> ((T) 479249);
    const auto m277920720 = juce::dsp::SIMDRegister<T> ((T) 277920720);
    const auto m3177720 = juce::dsp::SIMDRegister<T> ((T) 3177720);
    const auto m18361 = juce::dsp::SIMDRegister<T> ((T) 18361);
    const auto mnegone = juce::dsp::SIMDRegister<T> ((T) -1);

    auto x2 = x * x;
    auto num = mnegone * (x * ((x2 * (m1640635920 + (x2 * ((x2 * m479249) - m52785432)))) - m11511339840));
    auto den = m11511339840 + (x2 * (m277920720 + (x2 * (m3177720 + (x2 * m18361)))));

    return num / den;
}

/**
 * Fast implementation of the JUCE
 * FastMathApproximations::cos function
 * with SIMD optimisation.
 */
template <typename T>
inline juce::dsp::SIMDRegister<T> fastcosSIMD (juce::dsp::SIMDRegister<T> x) noexcept
{
    const auto m39251520 = juce::dsp::SIMDRegister<T> ((T) 39251520);
    const auto m18471600 = juce::dsp::SIMDRegister<T> ((T) 18471600);
    const auto m1075032 = juce::dsp::SIMDRegister<T> ((T) 1075032);
    const auto m14615 = juce::dsp::SIMDRegister<T> ((T) 14615);
    const auto m1154160 = juce::dsp::SIMDRegister<T> ((T) 1154160);
    const auto m16632 = juce::dsp::SIMDRegister<T> ((T) 16632);
    const auto m127 = juce::dsp::SIMDRegister<T> ((T) 127);

    auto x2 = x * x;
    auto num = m39251520 - (x2 * (m18471600 + (x2 * ((m14615 * x2) - m1075032))));
    auto den = m39251520 + (x2 * (m1154160 + (x2 * (m16632 + (x2 * m127)))));

    return num / den;
}

/** Clamps the input value to the range (-pi, pi) */
template <typename T>
inline juce::dsp::SIMDRegister<T> clampToPiRangeSIMD (juce::dsp::SIMDRegister<T> x)
{
    const auto mpi = juce::dsp::SIMDRegister<T> (juce::MathConstants<T>::pi);
    const auto m2pi = juce::dsp::SIMDRegister<T> (juce::MathConstants<T>::twoPi);
    const auto oo2p = juce::dsp::SIMDRegister<T> ((T) 1.0 / juce::MathConstants<T>::twoPi);
    const auto mz = juce::dsp::SIMDRegister<T> ((T) 0);

    auto y = x + mpi;
    auto yip = juce::dsp::SIMDRegister<T>::truncate (y * oo2p);
    auto p = y - (m2pi * yip);
    auto off = m2pi & juce::dsp::SIMDRegister<T>::lessThan (p, mz);
    p = p + off;
    return p - mpi;
}
} // namespace chowdsp::SIMDUtils
