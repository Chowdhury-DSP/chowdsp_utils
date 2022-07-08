#pragma once

#include <catch2/catch2.hpp>

#if JUCE_MODULE_AVAILABLE_chowdsp_simd
#include <chowdsp_simd/chowdsp_simd.h>
#endif

#if JUCE_MODULE_AVAILABLE_chowdsp_dsp_data_structures
#include <chowdsp_dsp_data_structures/chowdsp_dsp_data_structures.h>
#endif

// Helper for using Catch's REQUIRE with a custom failure message
#define REQUIRE_MESSAGE(cond, msg) \
    do                             \
    {                              \
        INFO (msg);                \
        REQUIRE (cond);            \
    } while ((void) 0, 0)

#if JUCE_MODULE_AVAILABLE_chowdsp_simd
template <typename T>
struct SIMDApproxImpl
{
    explicit SIMDApproxImpl (xsimd::batch<T> const& value) : m_value (value)
    {
    }

    SIMDApproxImpl& margin (T const& newMargin)
    {
        m_margin = newMargin;
        return *this;
    }

    friend bool operator== (const xsimd::batch<T>& lhs, SIMDApproxImpl const& rhs)
    {
        bool result = true;
        for (size_t i = 0; i < xsimd::batch<T>::size; ++i)
            result &= (lhs.get (i) == Approx (rhs.m_value.get (0)).margin (rhs.m_margin));

        return result;
    }

private:
    xsimd::batch<T> m_value;
    T m_margin;
};

template <typename T>
using SIMDApprox = std::conditional_t<chowdsp::SampleTypeHelpers::IsSIMDRegister<std::remove_const_t<T>>, SIMDApproxImpl<chowdsp::SampleTypeHelpers::NumericType<T>>, Approx>;
#endif

namespace test_utils
{
#if JUCE_MODULE_AVAILABLE_chowdsp_dsp_data_structures
template <typename FloatType = float, typename NumericType = chowdsp::SampleTypeHelpers::NumericType<FloatType>>
inline auto makeSineWave (NumericType frequency, NumericType sampleRate, int lengthSamples)
{
    chowdsp::Buffer<FloatType> sineBuffer (1, lengthSamples);

    auto* x = sineBuffer.getWritePointer (0);
    for (int n = 0; n < lengthSamples; ++n)
        x[n] = std::sin (juce::MathConstants<NumericType>::twoPi * frequency * (NumericType) n / sampleRate);

    return std::move (sineBuffer);
}

template <typename FloatType = float, typename NumericType = chowdsp::SampleTypeHelpers::NumericType<FloatType>>
inline auto makeSineWave (NumericType frequency, NumericType sampleRate, NumericType lengthSeconds)
{
    const int lengthSamples = int (lengthSeconds * sampleRate);
    return makeSineWave<FloatType> (frequency, sampleRate, lengthSamples);
}

template <typename FloatType = float, typename NumericType = chowdsp::SampleTypeHelpers::NumericType<FloatType>>
inline auto makeNoise (int numSamples, int numChannels = 1)
{
    std::random_device rd;
    std::mt19937 mt (rd());
    std::uniform_real_distribution<float> minus1To1 (-1.0f, 1.0f);

    chowdsp::Buffer<FloatType> noiseBuffer (numChannels, numSamples);

    for (int ch = 0; ch < numChannels; ++ch)
    {
        auto* x = noiseBuffer.getWritePointer (ch);
        for (int n = 0; n < numSamples; ++n)
            x[n] = (FloatType) minus1To1 (mt);
    }

    return std::move (noiseBuffer);
}
#endif
} // namespace test_utils
