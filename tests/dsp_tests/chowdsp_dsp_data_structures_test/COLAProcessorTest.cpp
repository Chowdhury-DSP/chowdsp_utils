#include <TimedUnitTest.h>
#include <test_utils.h>
#include <chowdsp_dsp_data_structures/chowdsp_dsp_data_structures.h>

namespace Constants
{
constexpr int fftOrder = 9;
constexpr int fftSize = 1 << fftOrder;

constexpr double fs = 48000.0f;
} // namespace Constants

class COLAProcessorTest : public TimedUnitTest
{
public:
    COLAProcessorTest() : TimedUnitTest ("COLA Processor Test")
    {
        proc.ut = this;
        proc.prepare ({ Constants::fs, (juce::uint32) Constants::fftSize * 2, 1 });
    }

    struct TestProcessor : public chowdsp::COLAProcessor<float>
    {
        TestProcessor() : chowdsp::COLAProcessor<float> (Constants::fftOrder) {}

        void processFrame (juce::AudioBuffer<float>& buffer) override
        {
            ut->expectEquals (buffer.getNumChannels(), 1, "Number of channels is incorrect!");
            ut->expectEquals (buffer.getNumSamples(), Constants::fftSize, "Block size is incorrect!");
        }

        TimedUnitTest* ut = nullptr;
    };

    void colaTest (int inputBlockSize)
    {
        constexpr int numBlocks = 20;
        auto buffer = test_utils::makeSineWave (100.0f, (float) Constants::fs, inputBlockSize * numBlocks);

        proc.reset();
        juce::AudioBuffer<float> refBuffer (1, buffer.getNumSamples());
        refBuffer.clear();
        refBuffer.copyFrom (0, Constants::fftSize, buffer, 0, 0, buffer.getNumSamples() - Constants::fftSize);

        for (int i = 0; i < numBlocks; ++i)
        {
            juce::AudioBuffer<float> procBuffer (buffer.getArrayOfWritePointers(), 1, inputBlockSize * i, inputBlockSize);
            proc.processBlock (procBuffer);
        }

        for (int i = Constants::fftSize * 2; i < buffer.getNumSamples() - Constants::fftSize; i += 20)
        {
            auto actual = buffer.getSample (0, i);
            auto expected = refBuffer.getSample (0, i);
            expectWithinAbsoluteError (actual, expected, 2.0e-2f, "Output sample is incorrect! At index " + juce::String (i));
        }
    }

    void runTestTimed() override
    {
        beginTest ("Same Sized Blocks Test");
        colaTest (Constants::fftSize);

        beginTest ("Shorter Blocks Test");
        colaTest (Constants::fftSize - 200);

        beginTest ("Longer Blocks Test");
        colaTest (Constants::fftSize + 200);
    }

    TestProcessor proc;
};

static COLAProcessorTest colaProcessorTest;
