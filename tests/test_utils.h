#pragma once

#include <JuceHeader.h>

namespace test_utils
{
JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wpessimizing-move") // Clang doesn't like std::move

inline AudioBuffer<float> makeSineWave (float frequency, float sampleRate, float lengthSeconds)
{
    const int lengthSamples = int (lengthSeconds * sampleRate);
    AudioBuffer<float> sineBuffer (1, lengthSamples);

    for (int n = 0; n < lengthSamples; ++n)
        sineBuffer.setSample (0, n, std::sin (MathConstants<float>::twoPi * frequency * (float) n / sampleRate));

    return std::move (sineBuffer);
}

inline AudioBuffer<float> makeImpulse (float amplitude, float sampleRate, float lengthSeconds)
{
    const int lengthSamples = int (lengthSeconds * sampleRate);
    AudioBuffer<float> impBuffer (1, lengthSamples);
    impBuffer.clear();

    impBuffer.setSample (0, 0, amplitude);

    return std::move (impBuffer);
}

inline AudioBuffer<float> makeNoise (float sampleRate, float lengthSeconds)
{
    const int lengthSamples = int (lengthSeconds * sampleRate);
    AudioBuffer<float> noiseBuffer (1, lengthSamples);

    Random rand;
    for (int n = 0; n < lengthSamples; ++n)
        noiseBuffer.setSample (0, n, (rand.nextFloat() - 0.5f) * 2.0f);

    return std::move (noiseBuffer);
}

JUCE_END_IGNORE_WARNINGS_GCC_LIKE

} // namespace test_utils
