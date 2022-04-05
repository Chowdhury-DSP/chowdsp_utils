#pragma once

#include <JuceHeader.h>

namespace test_utils
{
JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wpessimizing-move") // Clang doesn't like std::move

template <typename FloatType = float>
inline AudioBuffer<FloatType> makeSineWave (FloatType frequency, FloatType sampleRate, FloatType lengthSeconds)
{
    const int lengthSamples = int (lengthSeconds * sampleRate);
    AudioBuffer<FloatType> sineBuffer (1, lengthSamples);

    for (int n = 0; n < lengthSamples; ++n)
        sineBuffer.setSample (0, n, std::sin (MathConstants<FloatType>::twoPi * frequency * (FloatType) n / sampleRate));

    return std::move (sineBuffer);
}

template <typename FloatType = float>
inline AudioBuffer<FloatType> makeSineWave (FloatType frequency, FloatType sampleRate, int lengthSamples)
{
    AudioBuffer<FloatType> sineBuffer (1, lengthSamples);

    for (int n = 0; n < lengthSamples; ++n)
        sineBuffer.setSample (0, n, std::sin (MathConstants<FloatType>::twoPi * frequency * (FloatType) n / sampleRate));

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

inline AudioBuffer<float> makeNoise (juce::Random& rand, int numSamples, int numChannels = 1)
{
    AudioBuffer<float> noiseBuffer (numChannels, numSamples);

    for (int ch = 0; ch < numChannels; ++ch)
        for (int n = 0; n < numSamples; ++n)
            noiseBuffer.setSample (ch, n, (rand.nextFloat() - 0.5f) * 2.0f);

    return std::move (noiseBuffer);
}

inline AudioBuffer<float> makeNoise (float sampleRate, float lengthSeconds)
{
    const int lengthSamples = int (lengthSeconds * sampleRate);
    Random rand;

    return makeNoise (rand, lengthSamples);
}

/** Convert from a AudioBuffer to AudioBlock (maybe changing data type...) */
template <typename T, typename NumericType = typename chowdsp::SampleTypeHelpers::ElementType<T>::Type>
inline dsp::AudioBlock<T> bufferToBlock (HeapBlock<char>& dataBlock, const AudioBuffer<NumericType>& buffer)
{
    dsp::AudioBlock<T> block { dataBlock, 1, (size_t) buffer.getNumSamples() };
    for (int i = 0; i < buffer.getNumSamples(); ++i)
        block.setSample (0, i, buffer.getSample (0, i));

    return std::move (block);
}

/** Convert from a AudioBlock to AudioBuffer (maybe changing data type...) */
template <typename T, typename NumericType = typename chowdsp::SampleTypeHelpers::ElementType<T>::Type>
inline void blockToBuffer (AudioBuffer<NumericType>& buffer, const dsp::AudioBlock<T>& block)
{
    if constexpr (std::is_floating_point<T>::value)
    {
        for (int i = 0; i < buffer.getNumSamples(); ++i)
            buffer.setSample (0, i, block.getSample (0, i));
    }
    else
    {
        for (int i = 0; i < buffer.getNumSamples(); ++i)
            buffer.setSample (0, i, block.getSample (0, i).get (0));
    }
}
JUCE_END_IGNORE_WARNINGS_GCC_LIKE

struct ScopedFile
{
    explicit ScopedFile (const String& name) : file (File::getSpecialLocation (File::userHomeDirectory).getChildFile (name))
    {
    }

    explicit ScopedFile (const File& thisFile) : file (thisFile)
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

inline auto createDummyMouseEvent (Component* comp, ModifierKeys mods = {})
{
    auto mouseSource = Desktop::getInstance().getMainMouseSource();
    return MouseEvent { mouseSource, Point<float> {}, mods, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, comp, comp, Time::getCurrentTime(), Point<float> {}, Time::getCurrentTime(), 1, false };
}

} // namespace test_utils
