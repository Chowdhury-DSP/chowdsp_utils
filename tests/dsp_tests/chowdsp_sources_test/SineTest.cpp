#include "CatchUtils.h"
#include <chowdsp_sources/chowdsp_sources.h>

namespace
{
constexpr double fs = 44100.0;
constexpr double freq1 = 100.0;
constexpr double freq2 = 200.0;
constexpr double err = 1.0e-3;
} // namespace

/** Unit tests for chowdsp::SineWave. Testing accuracy compared to std::sin for:
 *  - single samples
 *  - buffers
*/
TEST_CASE ("Sine Test", "[dsp][sources]")
{
    SECTION ("Single-Sample Processing Test")
    {
        chowdsp::SineWave<double> chowSine;
        juce::dsp::ProcessSpec spec { fs, 512, 1 };
        chowSine.prepare (spec);
        chowSine.setFrequency (freq1);

        // process samples one at a time, and compare to std::sin
        for (int i = 0; i < 2000; i++)
        {
            auto actual = chowSine.processSample();
            auto expected = std::sin (juce::MathConstants<double>::twoPi * freq1 * i / fs);
            REQUIRE_MESSAGE (actual == Catch::Approx (expected).margin (err), "Sine Wave is inaccurate");
        }
    }

    SECTION ("Single-Sample Quadrature Processing Test")
    {
        chowdsp::SineWave<double> chowSine;
        juce::dsp::ProcessSpec spec { fs, 512, 1 };
        chowSine.prepare (spec);
        chowSine.setFrequency (freq1);

        // process samples one at a time, and compare to std::sin and std::cos
        for (int i = 0; i < 2000; i++)
        {
            auto [actual_sin, actual_cos] = chowSine.processSampleQuadrature();
            auto expected_sin = std::sin (juce::MathConstants<double>::twoPi * freq1 * i / fs);
            auto expected_cos = std::cos (juce::MathConstants<double>::twoPi * freq1 * i / fs);
            REQUIRE_MESSAGE (actual_sin == Catch::Approx (expected_sin).margin (err), "Sine Wave is inaccurate");
            REQUIRE_MESSAGE (actual_cos == Catch::Approx (expected_cos).margin (10.0 * err), "Cosine Wave is inaccurate");
        }
    }

    SECTION ("Buffer Processing Test")
    {
        chowdsp::SineWave<double> chowSine;

        constexpr int blockSize = 512;
        constexpr int numChannels = 4;
        juce::dsp::ProcessSpec spec { fs, blockSize, numChannels };
        chowSine.prepare (spec);
        chowSine.setFrequency (freq2);

        chowdsp::Buffer<double> chowBuffer (numChannels, 2 * blockSize);
        chowdsp::Buffer<double> refBuffer (numChannels, 2 * blockSize);

        chowBuffer.clear();
        refBuffer.clear();

        // process reference buffer
        for (int ch = 0; ch < numChannels; ++ch)
            for (int i = 0; i < refBuffer.getNumSamples(); ++i)
                refBuffer.getWritePointer (ch)[i] = std::sin (juce::MathConstants<double>::twoPi * freq2 * i / fs);

        // process with chowdsp::SineWave
        for (int i = 0; i < 2; ++i)
        {
            chowdsp::BufferView<double> block { chowBuffer, i * blockSize, blockSize };
            chowSine.processBlock (block);
        }

        // check for accuracy
        for (int ch = 0; ch < numChannels; ++ch)
        {
            for (int i = 0; i < 2 * blockSize; ++i)
            {
                REQUIRE_MESSAGE (chowBuffer.getReadPointer (ch)[i] == Catch::Approx (refBuffer.getReadPointer (ch)[i]).margin (err), "Sine Wave is inaccurate");
            }
        }
    }

    //    SECTION ("Channel Layouts Test")
    //    {
    //        constexpr float errF = 1.0e-3f;
    //        chowdsp::SineWave<float> chowSine;
    //
    //        constexpr int blockSize = (int) fs;
    //        constexpr int numChannels1 = 1;
    //        constexpr int numChannels2 = 2;
    //        juce::dsp::ProcessSpec spec { fs, blockSize, numChannels2 };
    //        chowSine.prepare (spec);
    //        chowSine.setFrequency (1.0f);
    //
    //        juce::AudioBuffer<float> buffer1 (numChannels1, blockSize);
    //        juce::AudioBuffer<float> buffer2 (numChannels2, blockSize);
    //
    //        buffer1.clear();
    //        buffer2.clear();
    //
    //        // initial run
    //        {
    //            juce::dsp::AudioBlock<float> block (buffer1);
    //            juce::dsp::ProcessContextReplacing<float> context (block);
    //            chowSine.process (context);
    //            expectWithinAbsoluteError (buffer1.getMagnitude (0, blockSize), 1.0f, errF, "Filling original buffer incorrect!");
    //        }
    //
    //        // 2 channels -> 4 channels
    //        {
    //            juce::dsp::AudioBlock<float> block1 (buffer1);
    //            juce::dsp::AudioBlock<float> block2 (buffer2);
    //            juce::dsp::ProcessContextNonReplacing<float> context (block1, block2);
    //            chowSine.process (context);
    //
    //            expectWithinAbsoluteError (buffer1.getMagnitude (0, blockSize), 1.0f, errF, "Out-of-place processing incorrect (original buffer)!");
    //            expectWithinAbsoluteError (buffer2.getMagnitude (0, 0, blockSize), 2.0f, errF, "Adding to original data incorrect!");
    //            expectWithinAbsoluteError (buffer2.getMagnitude (1, 0, blockSize), 1.0f, errF, "Filling new data incorrect!");
    //        }
    //    }
}
