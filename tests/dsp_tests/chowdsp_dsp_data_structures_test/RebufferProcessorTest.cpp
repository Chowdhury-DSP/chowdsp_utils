#include <CatchUtils.h>
#include <chowdsp_dsp_data_structures/chowdsp_dsp_data_structures.h>

namespace
{
constexpr int rebufferedBlockSize = 512;
}

struct TestProcessor : public chowdsp::RebufferedProcessor<float>
{
    int prepareRebuffering (const juce::dsp::ProcessSpec&) override
    {
        return rebufferedBlockSize;
    }

    void processRebufferedBlock (const chowdsp::BufferView<float>& buffer) override
    {
        REQUIRE_MESSAGE (buffer.getNumChannels() == 1, "Number of channels is incorrect!");
        REQUIRE_MESSAGE (buffer.getNumSamples() == rebufferedBlockSize, "Block size is incorrect!");

        chowdsp::BufferMath::applyGain (buffer, 2.0f);
    }
};

static void rebufferTest (int inputBlockSize, bool fewerChannels = false)
{
    constexpr double fs = 48000.0f;
    constexpr int numBlocks = 10;
    auto buffer = test_utils::makeSineWave (100.0f, (float) fs, inputBlockSize * numBlocks);

    TestProcessor proc;
    juce::dsp::ProcessSpec spec { fs, (juce::uint32) inputBlockSize, juce::uint32 (fewerChannels ? 2 : 1) };
    proc.prepare (spec);

    chowdsp::Buffer<float> refBuffer (1, buffer.getNumSamples());
    refBuffer.clear();
    chowdsp::BufferMath::copyBufferData (buffer, refBuffer, 0, proc.getLatencySamples(), buffer.getNumSamples() - proc.getLatencySamples());
    chowdsp::BufferMath::applyGain (refBuffer, 2.0f);

    for (int i = 0; i < numBlocks; ++i)
        proc.processBlock (chowdsp::BufferView<float> { buffer, inputBlockSize * i, inputBlockSize });

    for (int i = 0; i < buffer.getNumSamples(); i += 20)
    {
        auto actual = buffer.getReadPointer (0)[i];
        auto expected = refBuffer.getReadPointer (0)[i];
        REQUIRE_MESSAGE (actual == Catch::Approx (expected).margin (1.0e-6f), "Output sample is incorrect! At index " << std::to_string (i));
    }
}

TEST_CASE ("Rebuffer Test", "[dsp][data-structures]")
{
    SECTION ("Same Sized Blocks Test")
    {
        rebufferTest (rebufferedBlockSize);
    }

    SECTION ("Shorter Blocks Test")
    {
        rebufferTest (rebufferedBlockSize - 200);
    }

    SECTION ("Longer Blocks Test")
    {
        rebufferTest (rebufferedBlockSize + 200);
    }

    SECTION ("Fewer Channels Test")
    {
        rebufferTest (rebufferedBlockSize, true);
    }
}
