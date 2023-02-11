#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>

#if JUCE_MODULE_AVAILABLE_chowdsp_dsp_data_structures
#include <chowdsp_dsp_data_structures/chowdsp_dsp_data_structures.h>
#endif

namespace test_utils
{
namespace juce_utils
{
    template <typename FloatType = float>
    inline juce::AudioBuffer<FloatType> makeSineWave (FloatType frequency, FloatType sampleRate, FloatType lengthSeconds)
    {
        const int lengthSamples = int (lengthSeconds * sampleRate);
        juce::AudioBuffer<FloatType> sineBuffer (1, lengthSamples);

        for (int n = 0; n < lengthSamples; ++n)
            sineBuffer.setSample (0, n, std::sin (juce::MathConstants<FloatType>::twoPi * frequency * (FloatType) n / sampleRate));

        return sineBuffer;
    }

    template <typename FloatType = float>
    inline juce::AudioBuffer<FloatType> makeSineWave (FloatType frequency, FloatType sampleRate, int lengthSamples)
    {
        juce::AudioBuffer<FloatType> sineBuffer (1, lengthSamples);

        for (int n = 0; n < lengthSamples; ++n)
            sineBuffer.setSample (0, n, std::sin (juce::MathConstants<FloatType>::twoPi * frequency * (FloatType) n / sampleRate));

        return sineBuffer;
    }

    inline juce::AudioBuffer<float> makeImpulse (float amplitude, float sampleRate, float lengthSeconds)
    {
        const int lengthSamples = int (lengthSeconds * sampleRate);
        juce::AudioBuffer<float> impBuffer (1, lengthSamples);
        impBuffer.clear();

        impBuffer.setSample (0, 0, amplitude);

        return impBuffer;
    }
} // namespace juce_utils

#if JUCE_MODULE_AVAILABLE_chowdsp_dsp_data_structures
/** Convert from a AudioBuffer to AudioBlock (maybe changing data type...) */
template <typename T, typename NumericType = chowdsp::SampleTypeHelpers::NumericType<T>>
inline chowdsp::AudioBlock<T> bufferToBlock (juce::HeapBlock<char>& dataBlock, const juce::AudioBuffer<NumericType>& buffer)
{
    chowdsp::AudioBlock<T> block { dataBlock, 1, (size_t) buffer.getNumSamples() };
    for (int i = 0; i < buffer.getNumSamples(); ++i)
        block.setSample (0, i, buffer.getSample (0, i));

    return block;
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

struct ScopedFile
{
    explicit ScopedFile (const juce::String& name) : file (juce::File::getSpecialLocation (juce::File::userHomeDirectory).getChildFile (name))
    {
        if (file.existsAsFile())
            file.deleteFile();
    }

    explicit ScopedFile (const juce::File& thisFile) : file (thisFile)
    {
    }

    ~ScopedFile()
    {
        file.deleteRecursively();
    }

    operator juce::File() { return file; } // NOLINT
    operator const juce::File() const { return file; } // NOLINT

    const juce::File file;
};

inline void setParameter (juce::AudioProcessorParameter* param, float value)
{
    param->setValueNotifyingHost (value);
    juce::MessageManager::getInstance()->runDispatchLoopUntil (250);
}
} // namespace test_utils
