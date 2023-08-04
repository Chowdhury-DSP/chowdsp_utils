#include <CatchUtils.h>
#include <catch2/generators/catch_generators_all.hpp>
#include <chowdsp_dsp_data_structures/chowdsp_dsp_data_structures.h>

template <typename BufferViewType>
void testBufferView (const BufferViewType& bufferView, const juce::AudioBuffer<float>& buffer)
{
    REQUIRE_MESSAGE (bufferView.getNumChannels() == buffer.getNumChannels(), "Number of channels is incorrect!");
    REQUIRE_MESSAGE (bufferView.getNumSamples() == buffer.getNumSamples(), "Number of samples is incorrect!");

    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        auto* x = buffer.getReadPointer (ch);
        auto* xView = bufferView.getReadPointer (ch);
        for (int n = 0; n < buffer.getNumSamples(); ++n)
            REQUIRE_MESSAGE (juce::exactlyEqual (xView[n], x[n]), "Sample " + juce::String (n) + " is incorrect");
    }
}

template <typename T>
static void processBufferView (const chowdsp::BufferView<T>& buffer)
{
    const auto numChannels = buffer.getNumChannels();
    const auto numSamples = buffer.getNumSamples();
    for (int ch = 0; ch < numChannels; ++ch)
    {
        auto* x = buffer.getWritePointer (ch);
        juce::FloatVectorOperations::multiply (x, (T) 2, numSamples);
    }
}

TEST_CASE ("JUCE Buffer View Test", "[dsp][buffers]")
{
    const auto rand = Catch::Generators::random (-1.0f, 1.0f);

    SECTION ("juce::AudioBuffer to BufferView Test")
    {
        juce::AudioBuffer<float> juceBuffer (2, 128);
        for (int ch = 0; ch < juceBuffer.getNumChannels(); ++ch)
        {
            auto* x = juceBuffer.getWritePointer (ch);
            for (int n = 0; n < juceBuffer.getNumSamples(); ++n)
                x[n] = rand.get();
        }

        const auto constJuceBuffer = juce::AudioBuffer<float> { juceBuffer };
        testBufferView<chowdsp::BufferView<float>> (juceBuffer, constJuceBuffer);
        testBufferView<chowdsp::BufferView<const float>> (juceBuffer, constJuceBuffer);
        testBufferView<chowdsp::BufferView<const float>> (constJuceBuffer, constJuceBuffer);
    }

    SECTION ("juce::AudioBlock to BufferView Test")
    {
        juce::AudioBuffer<float> juceBuffer (2, 128);
        for (int ch = 0; ch < juceBuffer.getNumChannels(); ++ch)
        {
            auto* x = juceBuffer.getWritePointer (ch);
            for (int n = 0; n < juceBuffer.getNumSamples(); ++n)
                x[n] = rand.get();
        }

        juce::dsp::AudioBlock<float> block { juceBuffer };
        auto constBlock = static_cast<juce::dsp::AudioBlock<const float>> (block);

        testBufferView<chowdsp::BufferView<float>> (block, juceBuffer);
        testBufferView<chowdsp::BufferView<const float>> (block, juceBuffer);
        testBufferView<chowdsp::BufferView<const float>> (constBlock, juceBuffer);
    }

    SECTION ("Process test")
    {
        juce::AudioBuffer<float> juceBuffer (2, 128);
        for (int ch = 0; ch < juceBuffer.getNumChannels(); ++ch)
        {
            auto* x = juceBuffer.getWritePointer (ch);
            juce::FloatVectorOperations::fill (x, 1.0f, juceBuffer.getNumSamples());
        }

        processBufferView<float> (juceBuffer);

        juce::dsp::AudioBlock<float> block { juceBuffer };
        processBufferView<float> (block);

        for (int ch = 0; ch < juceBuffer.getNumChannels(); ++ch)
        {
            auto* x = juceBuffer.getReadPointer (ch);
            for (int n = 0; n < juceBuffer.getNumSamples(); ++n)
                REQUIRE_MESSAGE (juce::exactlyEqual (x[n], 4.0f), "Sample " + juce::String (n) + " is incorrect");
        }
    }
}
