#include <TimedUnitTest.h>
#include <chowdsp_dsp_data_structures/chowdsp_dsp_data_structures.h>

class BufferConversionTest : public TimedUnitTest
{
public:
    BufferConversionTest() : TimedUnitTest ("Buffer Conversion Test", "Buffers")
    {
    }

    template <typename BufferType>
    void toAudioBufferTest (BufferType& buffer)
    {
        auto juceBuffer = buffer.toAudioBuffer();
        expectEquals (juceBuffer.getNumChannels(), buffer.getNumChannels(), "Incorrect channel count!");
        expectEquals (juceBuffer.getNumSamples(), buffer.getNumSamples(), "Incorrect sample count!");
        expectEquals ((int64_t) juceBuffer.getWritePointer (0), (int64_t) buffer.getWritePointer (0), "Incorrect data!");

        const auto constJuceBuffer = std::as_const (buffer).toAudioBuffer();
        expectEquals (constJuceBuffer.getNumChannels(), buffer.getNumChannels(), "Incorrect channel count!");
        expectEquals (constJuceBuffer.getNumSamples(), buffer.getNumSamples(), "Incorrect sample count!");
        expectEquals ((int64_t) constJuceBuffer.getReadPointer (0), (int64_t) buffer.getReadPointer (0), "Incorrect data!");
    }

    template <typename BufferType>
    void toAudioBlockTest (BufferType& buffer)
    {
        auto juceBlock = buffer.toAudioBlock();
        expectEquals ((int) juceBlock.getNumChannels(), buffer.getNumChannels(), "Incorrect channel count!");
        expectEquals ((int) juceBlock.getNumSamples(), buffer.getNumSamples(), "Incorrect sample count!");
        expectEquals ((int64_t) juceBlock.getChannelPointer (0), (int64_t) buffer.getWritePointer (0), "Incorrect data!");

        const auto constJuceBlock = std::as_const (buffer).toAudioBlock();
        expectEquals ((int) constJuceBlock.getNumChannels(), buffer.getNumChannels(), "Incorrect channel count!");
        expectEquals ((int) constJuceBlock.getNumSamples(), buffer.getNumSamples(), "Incorrect sample count!");
        expectEquals ((int64_t) constJuceBlock.getChannelPointer (0), (int64_t) buffer.getReadPointer (0), "Incorrect data!");
    }

    void runTestTimed() override
    {
        chowdsp::Buffer<float> buffer { 2, 32 };
        chowdsp::BufferView<float> bufferView { buffer };
        chowdsp::StaticBuffer<float, 2, 32> staticBuffer { 2, 32 };

        beginTest ("chowdsp::Buffer -> juce::AudioBuffer");
        toAudioBufferTest (buffer);

        beginTest ("chowdsp::StaticBuffer -> juce::AudioBuffer");
        toAudioBufferTest (staticBuffer);

        beginTest ("chowdsp::BufferView -> juce::AudioBuffer");
        toAudioBufferTest (bufferView);

        beginTest ("chowdsp::Buffer -> juce::AudioBlock");
        toAudioBlockTest (buffer);

        beginTest ("chowdsp::StaticBuffer -> juce::AudioBlock");
        toAudioBlockTest (staticBuffer);

        beginTest ("chowdsp::BufferView -> juce::AudioBlock");
        toAudioBlockTest (bufferView);
    }
};

static BufferConversionTest bufferConversionTest;
