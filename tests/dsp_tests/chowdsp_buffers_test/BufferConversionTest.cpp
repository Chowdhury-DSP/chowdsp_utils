#include <CatchUtils.h>
#include <chowdsp_dsp_data_structures/chowdsp_dsp_data_structures.h>

template <typename BufferType>
void toAudioBufferTest (BufferType& buffer)
{
    auto juceBuffer = buffer.toAudioBuffer();
    REQUIRE_MESSAGE (juceBuffer.getNumChannels() == buffer.getNumChannels(), "Incorrect channel count!");
    REQUIRE_MESSAGE (juceBuffer.getNumSamples() == buffer.getNumSamples(), "Incorrect sample count!");
    REQUIRE_MESSAGE ((int64_t) juceBuffer.getWritePointer (0) == (int64_t) buffer.getWritePointer (0), "Incorrect data!");

    const auto constJuceBuffer = std::as_const (buffer).toAudioBuffer();
    REQUIRE_MESSAGE (constJuceBuffer.getNumChannels() == buffer.getNumChannels(), "Incorrect channel count!");
    REQUIRE_MESSAGE (constJuceBuffer.getNumSamples() == buffer.getNumSamples(), "Incorrect sample count!");
    REQUIRE_MESSAGE ((int64_t) constJuceBuffer.getReadPointer (0) == (int64_t) buffer.getReadPointer (0), "Incorrect data!");
}

template <typename BufferType>
void toAudioBlockTest (BufferType& buffer)
{
    auto juceBlock = buffer.toAudioBlock();
    REQUIRE_MESSAGE ((int) juceBlock.getNumChannels() == buffer.getNumChannels(), "Incorrect channel count!");
    REQUIRE_MESSAGE ((int) juceBlock.getNumSamples() == buffer.getNumSamples(), "Incorrect sample count!");
    REQUIRE_MESSAGE ((int64_t) juceBlock.getChannelPointer (0) == (int64_t) buffer.getWritePointer (0), "Incorrect data!");

    const auto constJuceBlock = std::as_const (buffer).toAudioBlock();
    REQUIRE_MESSAGE ((int) constJuceBlock.getNumChannels() == buffer.getNumChannels(), "Incorrect channel count!");
    REQUIRE_MESSAGE ((int) constJuceBlock.getNumSamples() == buffer.getNumSamples(), "Incorrect sample count!");
    REQUIRE_MESSAGE ((int64_t) constJuceBlock.getChannelPointer (0) == (int64_t) buffer.getReadPointer (0), "Incorrect data!");
}

TEST_CASE ("Buffer Conversion Test", "[dsp][buffers]")
{
    chowdsp::Buffer<float> buffer { 2, 32 };
    chowdsp::BufferView<float> bufferView { buffer };
    chowdsp::StaticBuffer<float, 2, 32> staticBuffer { 2, 32 };

    SECTION ("chowdsp::Buffer -> juce::AudioBuffer")
    {
        toAudioBufferTest (buffer);
    }

    SECTION ("chowdsp::StaticBuffer -> juce::AudioBuffer")
    {
        toAudioBufferTest (staticBuffer);
    }

    SECTION ("chowdsp::BufferView -> juce::AudioBuffer")
    {
        toAudioBufferTest (bufferView);
    }

    SECTION ("chowdsp::Buffer -> juce::AudioBlock")
    {
        toAudioBlockTest (buffer);
    }

    SECTION ("chowdsp::StaticBuffer -> juce::AudioBlock")
    {
        toAudioBlockTest (staticBuffer);
    }

    SECTION ("chowdsp::BufferView -> juce::AudioBlock")
    {
        toAudioBlockTest (bufferView);
    }
}
