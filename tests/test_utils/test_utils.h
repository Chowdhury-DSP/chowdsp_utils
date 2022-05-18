#pragma once

#include <juce_dsp/juce_dsp.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>

#if JUCE_MODULE_AVAILABLE_chowdsp_dsp_data_structures
#include <chowdsp_dsp_data_structures/chowdsp_dsp_data_structures.h>
#endif

namespace test_utils
{
JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wpessimizing-move") // Clang doesn't like std::move

template <typename FloatType = float>
inline juce::AudioBuffer<FloatType> makeSineWave (FloatType frequency, FloatType sampleRate, FloatType lengthSeconds)
{
    const int lengthSamples = int (lengthSeconds * sampleRate);
    juce::AudioBuffer<FloatType> sineBuffer (1, lengthSamples);

    for (int n = 0; n < lengthSamples; ++n)
        sineBuffer.setSample (0, n, std::sin (juce::MathConstants<FloatType>::twoPi * frequency * (FloatType) n / sampleRate));

    return std::move (sineBuffer);
}

template <typename FloatType = float>
inline juce::AudioBuffer<FloatType> makeSineWave (FloatType frequency, FloatType sampleRate, int lengthSamples)
{
    juce::AudioBuffer<FloatType> sineBuffer (1, lengthSamples);

    for (int n = 0; n < lengthSamples; ++n)
        sineBuffer.setSample (0, n, std::sin (juce::MathConstants<FloatType>::twoPi * frequency * (FloatType) n / sampleRate));

    return std::move (sineBuffer);
}

inline juce::AudioBuffer<float> makeImpulse (float amplitude, float sampleRate, float lengthSeconds)
{
    const int lengthSamples = int (lengthSeconds * sampleRate);
    juce::AudioBuffer<float> impBuffer (1, lengthSamples);
    impBuffer.clear();

    impBuffer.setSample (0, 0, amplitude);

    return std::move (impBuffer);
}

inline juce::AudioBuffer<float> makeNoise (juce::Random& rand, int numSamples, int numChannels = 1)
{
    juce::AudioBuffer<float> noiseBuffer (numChannels, numSamples);

    for (int ch = 0; ch < numChannels; ++ch)
        for (int n = 0; n < numSamples; ++n)
            noiseBuffer.setSample (ch, n, (rand.nextFloat() - 0.5f) * 2.0f);

    return std::move (noiseBuffer);
}

inline juce::AudioBuffer<float> makeNoise (float sampleRate, float lengthSeconds)
{
    const int lengthSamples = int (lengthSeconds * sampleRate);
    juce::Random rand;

    return makeNoise (rand, lengthSamples);
}

#if JUCE_MODULE_AVAILABLE_chowdsp_dsp_data_structures
/** Convert from a AudioBuffer to AudioBlock (maybe changing data type...) */
template <typename T, typename NumericType = chowdsp::SampleTypeHelpers::NumericType<T>>
inline chowdsp::AudioBlock<T> bufferToBlock (juce::HeapBlock<char>& dataBlock, const juce::AudioBuffer<NumericType>& buffer)
{
    chowdsp::AudioBlock<T> block { dataBlock, 1, (size_t) buffer.getNumSamples() };
    for (int i = 0; i < buffer.getNumSamples(); ++i)
        block.setSample (0, i, buffer.getSample (0, i));

    return std::move (block);
}

/** Convert from a AudioBlock to AudioBuffer (maybe changing data type...) */
template <typename T, typename NumericType = chowdsp::SampleTypeHelpers::NumericType<T>>
inline void blockToBuffer (juce::AudioBuffer<NumericType>& buffer, const chowdsp::AudioBlock<T>& block)
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
#endif // JUCE_MODULE_AVAILABLE_chowdsp_dsp_data_structures

JUCE_END_IGNORE_WARNINGS_GCC_LIKE

struct ScopedFile
{
    explicit ScopedFile (const juce::String& name) : file (juce::File::getSpecialLocation (juce::File::userHomeDirectory).getChildFile (name))
    {
    }

    explicit ScopedFile (const juce::File& thisFile) : file (thisFile)
    {
    }

    ~ScopedFile()
    {
        file.deleteRecursively();
    }

    const juce::File file;
};

inline void setParameter (juce::AudioProcessorParameter* param, float value)
{
    param->setValueNotifyingHost (value);
    juce::MessageManager::getInstance()->runDispatchLoopUntil (250);
}

inline auto createDummyMouseEvent (juce::Component* comp, juce::ModifierKeys mods = {})
{
    auto mouseSource = juce::Desktop::getInstance().getMainMouseSource();
    return juce::MouseEvent { mouseSource, juce::Point<float> {}, mods, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, comp, comp, juce::Time::getCurrentTime(), juce::Point<float> {}, juce::Time::getCurrentTime(), 1, false };
}

} // namespace test_utils
