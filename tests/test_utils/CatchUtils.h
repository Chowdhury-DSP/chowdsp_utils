#pragma once

#include <catch2/catch2.hpp>

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

namespace test_utils
{
#if JUCE_MODULE_AVAILABLE_chowdsp_dsp_data_structures
template <typename FloatType = float, typename NumericType = chowdsp::SampleTypeHelpers::NumericType<FloatType>>
inline auto makeSineWave (NumericType frequency, NumericType sampleRate, NumericType lengthSeconds)
{
    const int lengthSamples = int (lengthSeconds * sampleRate);
    chowdsp::Buffer<FloatType> sineBuffer (1, lengthSamples);

    auto* x = sineBuffer.getWritePointer (0);
    for (int n = 0; n < lengthSamples; ++n)
        x[n] = std::sin (juce::MathConstants<NumericType>::twoPi * frequency * (NumericType) n / sampleRate);

    return std::move (sineBuffer);
}
#endif
} // namespace test_utils
