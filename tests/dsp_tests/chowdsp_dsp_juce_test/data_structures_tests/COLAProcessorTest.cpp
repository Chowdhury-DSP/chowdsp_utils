#include <CatchUtils.h>
#include <chowdsp_dsp_data_structures/chowdsp_dsp_data_structures.h>

namespace Constants
{
constexpr int fftOrder = 9;
constexpr int fftSize = 1 << fftOrder;
constexpr double fs = 48000.0f;
} // namespace Constants

TEST_CASE ("COLA Processor Test", "[dsp][data-structures]")
{
    struct TestProcessor : public chowdsp::COLAProcessor<float>
    {
        TestProcessor() : chowdsp::COLAProcessor<float> (Constants::fftOrder) {}

        void processFrame (juce::AudioBuffer<float>& buffer) override
        {
            REQUIRE_MESSAGE (buffer.getNumChannels() == 1, "Number of channels is incorrect!");
            REQUIRE_MESSAGE (buffer.getNumSamples() == Constants::fftSize, "Block size is incorrect!");
        }
    };

    TestProcessor proc;
    proc.prepare ({ Constants::fs, (juce::uint32) Constants::fftSize * 2, 1 });

    const auto colaTest = [&proc] (int inputBlockSize)
    {
        constexpr int numBlocks = 20;
        auto buffer = test_utils::makeSineWave (100.0f, (float) Constants::fs, inputBlockSize * numBlocks);

        proc.reset();
        juce::AudioBuffer<float> refBuffer (1, buffer.getNumSamples());
        refBuffer.clear();
        chowdsp::BufferMath::copyBufferData (buffer, refBuffer, 0, Constants::fftSize, buffer.getNumSamples() - Constants::fftSize);

        for (int i = 0; i < numBlocks; ++i)
        {
            juce::AudioBuffer<float> procBuffer (buffer.getArrayOfWritePointers(), 1, inputBlockSize * i, inputBlockSize);
            proc.processBlock (procBuffer);
        }

        for (int i = Constants::fftSize * 2; i < buffer.getNumSamples() - Constants::fftSize; i += 20)
        {
            auto actual = buffer.getReadPointer (0)[i];
            auto expected = refBuffer.getSample (0, i);
            REQUIRE_MESSAGE (actual == Catch::Approx { expected }.margin (2.0e-2f), "Output sample is incorrect! At index " + juce::String (i));
        }
    };

    SECTION ("Same Sized Blocks Test")
    {
        colaTest (Constants::fftSize);
    }

    SECTION ("Shorter Blocks Test")
    {
        colaTest (Constants::fftSize - 200);
    }

    SECTION ("Longer Blocks Test")
    {
        colaTest (Constants::fftSize + 200);
    }
}
