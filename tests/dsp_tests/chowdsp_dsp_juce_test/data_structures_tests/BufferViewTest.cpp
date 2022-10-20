#include <TimedUnitTest.h>
#include <chowdsp_dsp_data_structures/chowdsp_dsp_data_structures.h>

class BufferViewTest : public TimedUnitTest
{
public:
    BufferViewTest() : TimedUnitTest ("Buffer View Test")
    {
    }

    void audioBufferToBufferViewTest (juce::Random& rand)
    {
        juce::AudioBuffer<float> juceBuffer (2, 128);
        for (int ch = 0; ch < juceBuffer.getNumChannels(); ++ch)
        {
            auto* x = juceBuffer.getWritePointer (ch);
            for (int n = 0; n < juceBuffer.getNumSamples(); ++n)
                x[n] = rand.nextFloat() * 2.0f - 1.0f;
        }

        chowdsp::BufferView view { juceBuffer };
        expectEquals (view.getNumChannels(), juceBuffer.getNumChannels(), "Number of channels is incorrect!");
        expectEquals (view.getNumSamples(), juceBuffer.getNumSamples(), "Number of samples is incorrect!");

        for (int ch = 0; ch < juceBuffer.getNumChannels(); ++ch)
        {
            auto* x = juceBuffer.getReadPointer (ch);
            auto* xView = view.getReadPointer (ch);
            for (int n = 0; n < juceBuffer.getNumSamples(); ++n)
                expectEquals (xView[n], x[n], "Sample is incorrect");
        }
    }

    void audioBlockToBufferViewTest (juce::Random& rand)
    {
        juce::AudioBuffer<float> juceBuffer (2, 128);
        for (int ch = 0; ch < juceBuffer.getNumChannels(); ++ch)
        {
            auto* x = juceBuffer.getWritePointer (ch);
            for (int n = 0; n < juceBuffer.getNumSamples(); ++n)
                x[n] = rand.nextFloat() * 2.0f - 1.0f;
        }

        juce::dsp::AudioBlock<float> block { juceBuffer };

        chowdsp::BufferView view { block };
        expectEquals (view.getNumChannels(), juceBuffer.getNumChannels(), "Number of channels is incorrect!");
        expectEquals (view.getNumSamples(), juceBuffer.getNumSamples(), "Number of samples is incorrect!");

        for (int ch = 0; ch < juceBuffer.getNumChannels(); ++ch)
        {
            auto* x = juceBuffer.getReadPointer (ch);
            auto* xView = view.getReadPointer (ch);
            for (int n = 0; n < juceBuffer.getNumSamples(); ++n)
                expectEquals (xView[n], x[n], "Sample is incorrect");
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

    void processTest()
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
                expectEquals (x[n], 4.0f, "Sample is incorrect");
        }
    }

    void runTestTimed() override
    {
        auto rand = getRandom();

        beginTest ("juce::AudioBuffer to BufferView Test");
        audioBufferToBufferViewTest (rand);

        beginTest ("juce::AudioBlock to BufferView Test");
        audioBlockToBufferViewTest (rand);

        beginTest ("Process test");
        processTest();
    }
};

static BufferViewTest bufferViewTest;
