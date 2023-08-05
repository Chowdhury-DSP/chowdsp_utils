#include <CatchUtils.h>
#include <chowdsp_dsp_utils/chowdsp_dsp_utils.h>

namespace
{
constexpr float fs = 48000.0f;
constexpr float freq = 100.0f;
constexpr int nSamples = 512;
constexpr float lenSeconds = (float) nSamples / fs;
} // namespace

TEST_CASE ("Panner Test", "[dsp][misc]")
{
    SECTION ("Buffer Test")
    {
        // set up panners
        chowdsp::Panner<float> panner;
        panner.setPan (-1.0f); // full left
        panner.setRule (chowdsp::Panner<float>::Rule::linear);
        panner.prepare ({ (double) fs, (juce::uint32) nSamples, 1 });

        // set up buffers
        auto buffer = test_utils::makeSineWave (freq, fs, lenSeconds, 2);

        // process
        panner.processBlock (buffer);
        auto leftMag = chowdsp::BufferMath::getMagnitude (buffer, 0, nSamples, 0);
        auto rightMag = chowdsp::BufferMath::getMagnitude (buffer, 0, nSamples, 1);

        REQUIRE (juce::approximatelyEqual (leftMag, 2.0f)); // expect both channels summed into left channel
        REQUIRE (juce::approximatelyEqual (rightMag, 0.0f)); // expect silence on right channel
    }

    SECTION ("Single-sample Test")
    {
        // set up panners
        chowdsp::Panner<float> panner;
        panner.setPan (1.0f); // full right
        panner.setRule (chowdsp::Panner<float>::Rule::linear);
        panner.prepare ({ (double) fs, (juce::uint32) nSamples, 1 });

        // set up buffers
        auto buffer = test_utils::makeSineWave (freq, fs, lenSeconds, 2);

        // process
        auto* input = buffer.getReadPointer (0);
        auto* left = buffer.getWritePointer (0);
        auto* right = buffer.getWritePointer (1);

        for (int i = 0; i < nSamples; ++i)
            std::tie (left[i], right[i]) = panner.processSample (input[i]);
        auto leftMag = chowdsp::BufferMath::getMagnitude (buffer, 0, nSamples, 0);
        auto rightMag = chowdsp::BufferMath::getMagnitude (buffer, 0, nSamples, 1);

        REQUIRE (juce::approximatelyEqual (leftMag, 0.0f)); // expect silenSecondsce on left channel
        REQUIRE (juce::approximatelyEqual (rightMag, 2.0f)); // expect both channels summed to right channel
    }

    SECTION ("Center Test")
    {
        // set up panners
        chowdsp::Panner<float> panner;
        panner.setPan (0.0f); // center
        panner.setRule (chowdsp::Panner<float>::Rule::squareRoot3dB);
        panner.prepare ({ (double) fs, (juce::uint32) nSamples, 1 });

        // set up buffers
        auto buffer = test_utils::makeSineWave (freq, fs, lenSeconds, 2);

        // process
        panner.processBlock (buffer);
        auto leftMag = chowdsp::BufferMath::getMagnitude (buffer, 0, nSamples, 0);
        auto rightMag = chowdsp::BufferMath::getMagnitude (buffer, 0, nSamples, 1);

        REQUIRE (leftMag == Catch::Approx (1.0f).margin (1.0e-3f));
        REQUIRE (rightMag == Catch::Approx (1.0f).margin (1.0e-3f));
    }
}
