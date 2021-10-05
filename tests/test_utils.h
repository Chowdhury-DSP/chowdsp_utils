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

struct ScopedFile
{
    ScopedFile (const String& name) : file (File::getSpecialLocation (File::userHomeDirectory).getChildFile (name))
    {
    }

    ScopedFile (const File& thisFile) : file (thisFile)
    {
    }

    ~ScopedFile()
    {
        file.deleteRecursively();
    }

    const File file;
};

inline void setParameter (AudioProcessorParameter* param, float value)
{
    param->setValueNotifyingHost (value);
    MessageManager::getInstance()->runDispatchLoopUntil (250);
}

} // namespace test_utils
