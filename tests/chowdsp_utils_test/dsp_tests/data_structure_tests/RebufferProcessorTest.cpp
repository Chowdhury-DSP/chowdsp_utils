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
        int prepareRebuffering (const juce::dsp::ProcessSpec&) override
        {
            return rebufferedBlockSize;
        }

        void processRebufferedBlock (AudioBuffer<float>& buffer) override
        {
            ut->expectEquals (buffer.getNumChannels(), 1, "Number of channels is incorrect!");
            ut->expectEquals (buffer.getNumSamples(), rebufferedBlockSize, "Block size is incorrect!");

            buffer.applyGain (2.0f);
        }

        TimedUnitTest* ut = nullptr;
    };

    void rebufferTest (int inputBlockSize, bool fewerChannels = false)
    {
        constexpr double fs = 48000.0f;
        constexpr int numBlocks = 10;
        auto buffer = test_utils::makeSineWave (100.0f, (float) fs, inputBlockSize * numBlocks);

        TestProcessor proc;
        proc.ut = this;

        dsp::ProcessSpec spec { fs, (uint32) inputBlockSize, uint32 (fewerChannels ? 2 : 1) };
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

        beginTest ("Fewer Channels Test");
        rebufferTest (rebufferedBlockSize, true);
    }
};

static RebufferedProcessorTest rebufferedProcessorTest;
