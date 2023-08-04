#include "CatchUtils.h"
#include <chowdsp_sources/chowdsp_sources.h>

namespace
{
constexpr auto _sampleRate = 1000.0;
constexpr auto _blockSize = 512;
constexpr auto testFreq = 100.0f;
} // namespace

TEST_CASE ("Square Test", "[dsp][sources]")
{
    SECTION ("Reference Test")
    {
        // our osc has 1/2 sample delay so, run the reference osc at 2x sample rate, and check every other.
        float phase = 0.0f;
        const auto phaseIncrement = juce::MathConstants<float>::twoPi * testFreq / float (2.0 * _sampleRate);
        auto refOsc = [&phase, phaseIncrement]() mutable
        {
            const auto y = (phase - juce::MathConstants<float>::pi) < 0.0f ? 1.0f : -1.0f;
            phase += phaseIncrement;

            while (phase >= juce::MathConstants<float>::twoPi)
                phase -= juce::MathConstants<float>::twoPi;

            return y;
        };

        chowdsp::SquareWave<float> testOsc;
        testOsc.prepare ({ _sampleRate, (juce::uint32) _blockSize, 1 });
        testOsc.setFrequency (testFreq);
        REQUIRE_MESSAGE (juce::exactlyEqual (testOsc.getFrequency(), testFreq), "Set frequency is incorrect!");

        testOsc.processSample(); // for half-sample delay
        for (int i = 0; i < 20; ++i)
        {
            refOsc();
            REQUIRE_MESSAGE (testOsc.processSample() == Catch::Approx (refOsc()).margin (0.01f), "Generated sample is incorrect!");
        }
    }

    SECTION ("SIMD Reference Test")
    {
        // our osc has 1/2 sample delay so, run the reference osc at 2x sample rate, and check every other.
        float phase = 0.0f;
        const auto phaseIncrement = juce::MathConstants<float>::twoPi * testFreq / float (2.0 * _sampleRate);
        auto refOsc = [&phase, phaseIncrement]() mutable
        {
            const auto y = (phase - juce::MathConstants<float>::pi) < 0.0f ? 1.0f : -1.0f;
            phase += phaseIncrement;

            while (phase >= juce::MathConstants<float>::twoPi)
                phase -= juce::MathConstants<float>::twoPi;

            return y;
        };

        chowdsp::SquareWave<xsimd::batch<float>> testOsc;
        testOsc.prepare ({ _sampleRate, (juce::uint32) _blockSize, 1 });
        testOsc.setFrequency (testFreq);
        REQUIRE_MESSAGE (juce::exactlyEqual (testOsc.getFrequency().get (0), testFreq), "Set frequency is incorrect!");

        testOsc.processSample(); // for half-sample delay
        for (int i = 0; i < 20; ++i)
        {
            refOsc();

            auto expOut = refOsc();
            auto testOut = testOsc.processSample();
            REQUIRE_MESSAGE (testOut.get (0) == Catch::Approx (expOut).margin (0.01f), "Generated sample is incorrect!");
            REQUIRE_MESSAGE (testOut.get (1) == Catch::Approx (expOut).margin (0.01f), "Generated sample is incorrect!");
        }
    }

    SECTION ("Process Replacing Test")
    {
        float phase = 0.0f;
        const auto phaseIncrement = juce::MathConstants<float>::twoPi * testFreq / float (2.0 * _sampleRate);
        auto refOsc = [&phase, phaseIncrement] (float input) mutable
        {
            const auto y = (phase - juce::MathConstants<float>::pi) < 0.0f ? 1.0f : -1.0f;
            phase += phaseIncrement;

            while (phase >= juce::MathConstants<float>::twoPi)
                phase -= juce::MathConstants<float>::twoPi;

            return y + input;
        };

        chowdsp::SquareWave<float> testOsc;
        testOsc.prepare ({ _sampleRate, (juce::uint32) _blockSize, 1 });
        testOsc.setFrequency (testFreq);

        chowdsp::Buffer<float> testBuffer (1, 21);
        juce::FloatVectorOperations::fill (testBuffer.getWritePointer (0), 1.0f, 21);
        testOsc.processBlock (testBuffer);

        for (int i = 0; i < 19; ++i)
        {
            refOsc (1.0f);
            auto expOut = refOsc (1.0f);
            auto actualOut = testBuffer.getReadPointer (0)[i + 1];
            REQUIRE_MESSAGE (actualOut == Catch::Approx (expOut).margin (0.01f), "Generated sample is incorrect!");
        }
    }

    SECTION ("Zero Hz Test")
    {
        chowdsp::SquareWave<float> testOsc;
        testOsc.prepare ({ _sampleRate, (juce::uint32) _blockSize, 1 });
        testOsc.setFrequency (0.0f);

        for (int i = 0; i < 10; ++i)
            REQUIRE_MESSAGE (juce::approximatelyEqual (testOsc.processSample(), 0.0f), "Zero Hz output is non-zero!");
    }
}
