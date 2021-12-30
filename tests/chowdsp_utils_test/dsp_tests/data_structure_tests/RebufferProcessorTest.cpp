#include <TimedUnitTest.h>
#include <test_utils.h>

namespace
{
constexpr int rebufferedBlockSize = 512;
}

class RebufferedProcessorTest : public TimedUnitTest
{
public:
    RebufferedProcessorTest() : TimedUnitTest ("Rebuffer Test") {}

    struct TestProcessor : public chowdsp::RebufferedProcessor<float>
    {
        int prepareRebuffering (const juce::dsp::ProcessSpec& spec) override
        {
            ignoreUnused (spec);
            return rebufferedBlockSize;
        }

        void processRebufferedBlock (AudioBuffer<float>& buffer) override
        {
            buffer.applyGain (2.0f);
        }
    };

    void rebufferTest (int inputBlockSize)
    {
        constexpr double fs = 48000.0f;
        constexpr int numBlocks = 10;
        auto buffer = test_utils::makeSineWave (100.0f, (float) fs, inputBlockSize * numBlocks);

        TestProcessor proc;
        dsp::ProcessSpec spec { fs, (uint32) inputBlockSize, 1 };
        proc.prepare (spec);

        AudioBuffer<float> refBuffer (1, buffer.getNumSamples());
        refBuffer.clear();
        refBuffer.copyFrom (0, proc.getLatencySamples(), buffer, 0, 0, buffer.getNumSamples() - proc.getLatencySamples());
        refBuffer.applyGain (2.0f);

        for (int i = 0; i < numBlocks; ++i)
        {
            AudioBuffer<float> procBuffer (buffer.getArrayOfWritePointers(), 1, inputBlockSize * i, inputBlockSize);
            proc.processBlock (procBuffer);
        }

        for (int i = 0; i < buffer.getNumSamples(); i += 20)
        {
            auto actual = buffer.getSample (0, i);
            auto expected = refBuffer.getSample (0, i);
            expectWithinAbsoluteError (actual, expected, 1.0e-6f, "Output sample is incorrect! At index " + String (i));
        }
    }

    void runTestTimed() override
    {
        beginTest ("Same Sized Blocks Test");
        rebufferTest (rebufferedBlockSize);

        beginTest ("Shorter Blocks Test");
        rebufferTest (rebufferedBlockSize - 200);

        beginTest ("Longer Blocks Test");
        rebufferTest (rebufferedBlockSize + 200);
    }
};

static RebufferedProcessorTest rebufferedProcessorTest;
