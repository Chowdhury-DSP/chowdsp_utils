#pragma once

#include <random>
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <catch2/generators/catch_generators_random.hpp>

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
            result &= (lhs.get (i) == Catch::Approx (rhs.m_value.get (0)).margin (rhs.m_margin));

        return result;
    }

private:
    xsimd::batch<T> m_value;
    T m_margin;
};

template <typename T>
using SIMDApprox = std::conditional_t<chowdsp::SampleTypeHelpers::IsSIMDRegister<std::remove_const_t<T>>, SIMDApproxImpl<chowdsp::SampleTypeHelpers::NumericType<T>>, Catch::Approx>;
#endif

namespace test_utils
{
template <typename FloatType>
struct RandomFloatGenerator
{
    Catch::Generators::RandomFloatingGenerator<FloatType> generator;

    RandomFloatGenerator (FloatType a, FloatType b)
        : generator (a, b, Catch::Generators::Detail::getSeed())
    {
    }

    FloatType operator()()
    {
        const auto y = generator.get();
        generator.next();
        return y;
    }
};

template <typename IntType>
struct RandomIntGenerator
{
    Catch::Generators::RandomIntegerGenerator<IntType> generator;

    RandomIntGenerator (IntType a, IntType b)
        : generator (a, b, Catch::Generators::Detail::getSeed())
    {
    }

    IntType operator()()
    {
        const auto y = generator.get();
        generator.next();
        return y;
    }
};

#if JUCE_MODULE_AVAILABLE_chowdsp_dsp_data_structures
template <typename FloatType = float, typename NumericType = chowdsp::SampleTypeHelpers::NumericType<FloatType>>
inline auto makeSineWave (NumericType frequency, NumericType sampleRate, int lengthSamples, int numChannels = 1)
{
    chowdsp::Buffer<FloatType> sineBuffer (numChannels, lengthSamples);

    auto* x = sineBuffer.getWritePointer (0);
    for (int n = 0; n < lengthSamples; ++n)
        x[n] = std::sin (juce::MathConstants<NumericType>::twoPi * frequency * (NumericType) n / sampleRate);

    for (int ch = 1; ch < numChannels; ++ch)
        chowdsp::BufferMath::copyBufferChannels (sineBuffer, sineBuffer, 0, ch);

    return sineBuffer;
}

template <typename FloatType = float, typename NumericType = chowdsp::SampleTypeHelpers::NumericType<FloatType>>
inline auto makeSineWave (NumericType frequency, NumericType sampleRate, NumericType lengthSeconds, int numChannels = 1)
{
    const int lengthSamples = int (lengthSeconds * sampleRate);
    return makeSineWave<FloatType> (frequency, sampleRate, lengthSamples, numChannels);
}

template <typename FloatType = float, typename NumericType = chowdsp::SampleTypeHelpers::NumericType<FloatType>>
inline auto makeNoise (int numSamples, int numChannels = 1)
{
    auto minus1To1 = RandomFloatGenerator (-1.0f, 1.0f);
    chowdsp::Buffer<FloatType> noiseBuffer (numChannels, numSamples);

    for (int ch = 0; ch < numChannels; ++ch)
    {
        auto* x = noiseBuffer.getWritePointer (ch);
        for (int n = 0; n < numSamples; ++n)
            x[n] = (FloatType) minus1To1();
    }

    return noiseBuffer;
}

inline chowdsp::Buffer<float> makeImpulse (float amplitude, float sampleRate, float lengthSeconds)
{
    const int lengthSamples = int (lengthSeconds * sampleRate);
    chowdsp::Buffer<float> impBuffer (1, lengthSamples);
    impBuffer.clear();

    impBuffer.getWritePointer (0)[0] = amplitude;

    return impBuffer;
}
#endif
} // namespace test_utils
