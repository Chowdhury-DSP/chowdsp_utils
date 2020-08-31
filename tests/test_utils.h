#pragma once

#include <JuceHeader.h>

namespace test_utils
{

AudioBuffer<float> makeSineWave (float frequency, float sampleRate, float lengthSeconds)
{
    const int lengthSamples = int (lengthSeconds * sampleRate);
    AudioBuffer<float> sineBuffer (1, lengthSamples);

    for (int n = 0; n < lengthSamples; ++n)
        sineBuffer.setSample (0, n, MathConstants<float>::twoPi * frequency * (float) n / sampleRate);

    return std::move (sineBuffer);
}

AudioBuffer<float> makeImpulse (float amplitude, float sampleRate, float lengthSeconds)
{
    const int lengthSamples = int (lengthSeconds * sampleRate);
    AudioBuffer<float> impBuffer (1, lengthSamples);
    impBuffer.clear();

    impBuffer.setSample (0, 0, amplitude);

    return std::move (impBuffer);
}

} // test-test_utils
