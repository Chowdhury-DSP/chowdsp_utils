#include "CatchUtils.h"
#include <chowdsp_sources/chowdsp_sources.h>

namespace
{
constexpr auto _sampleRate = 1000.0;
constexpr auto _blockSize = 512;

constexpr auto testFreq = 100.0f;
constexpr auto polygonOrder = 2.5f;
constexpr auto polygonTeeth = 1.25f;
} // namespace

static float polygonalCore (float phase, float order, float teeth)
{
    static constexpr auto pi = juce::MathConstants<float>::pi;
    const auto p = std::cos (pi / order) / std::cos ((1.0f / order) * std::fmod (phase * order, 2.0f * pi) - pi / order + teeth);
    return std::sin (phase) * p;
}

TEST_CASE ("Polygonal Test", "[dsp][sources]")
{
    SECTION ("Reference Test")
    {
        float phase = 0.0f;
        const auto phaseIncrement = juce::MathConstants<float>::twoPi * testFreq / float (_sampleRate);
        auto refOsc = [&phase, phaseIncrement]() mutable
        {
            const auto y = polygonalCore (phase, polygonOrder, polygonTeeth);

            phase += phaseIncrement;
            while (phase >= juce::MathConstants<float>::twoPi)
                phase -= juce::MathConstants<float>::twoPi;

            return y;
        };

        chowdsp::experimental::PolygonalOscillator<float> testOsc;
        testOsc.prepare ({ _sampleRate, (juce::uint32) _blockSize, 1 });
        testOsc.setFrequency (testFreq);
        testOsc.setOrder (polygonOrder);
        testOsc.setTeeth (polygonTeeth);
        REQUIRE_MESSAGE (juce::exactlyEqual (testOsc.getFrequency(), testFreq), "Set frequency is incorrect!");
        REQUIRE_MESSAGE (juce::exactlyEqual (testOsc.getOrder(), polygonOrder), "Set Order is incorrect!");
        REQUIRE_MESSAGE (juce::exactlyEqual (testOsc.getTeeth(), polygonTeeth), "Set Teeth is incorrect!");

        for (int i = 0; i < 20; ++i)
        {
            REQUIRE_MESSAGE (testOsc.processSample() == Catch::Approx (refOsc()).margin (0.01f), "Generated sample is incorrect!");
        }
    }

    SECTION ("SIMD Reference Test")
    {
        // our osc has 1/2 sample delay so, run the reference osc at 2x sample rate, and check every other.
        float phase = 0.0f;
        const auto phaseIncrement = juce::MathConstants<float>::twoPi * testFreq / float (_sampleRate);
        auto refOsc = [&phase, phaseIncrement]() mutable
        {
            const auto y = polygonalCore (phase, polygonOrder, polygonTeeth);

            phase += phaseIncrement;
            while (phase >= juce::MathConstants<float>::twoPi)
                phase -= juce::MathConstants<float>::twoPi;

            return y;
        };

        chowdsp::experimental::PolygonalOscillator<xsimd::batch<float>> testOsc;
        testOsc.prepare ({ _sampleRate, (juce::uint32) _blockSize, 1 });
        testOsc.setFrequency (testFreq);
        testOsc.setOrder (polygonOrder);
        testOsc.setTeeth (polygonTeeth);
        REQUIRE_MESSAGE (juce::exactlyEqual (testOsc.getFrequency().get (0), testFreq), "Set frequency is incorrect!");
        REQUIRE_MESSAGE (juce::exactlyEqual (testOsc.getOrder().get (0), polygonOrder), "Set Order is incorrect!");
        REQUIRE_MESSAGE (juce::exactlyEqual (testOsc.getTeeth().get (0), polygonTeeth), "Set Teeth is incorrect!");

        for (int i = 0; i < 20; ++i)
        {
            auto expOut = refOsc();
            auto testOut = testOsc.processSample();
            REQUIRE_MESSAGE (testOut.get (0) == Catch::Approx (expOut).margin (0.01f), "Generated sample is incorrect!");
            REQUIRE_MESSAGE (testOut.get (1) == Catch::Approx (expOut).margin (0.01f), "Generated sample is incorrect!");
        }
    }

    SECTION ("Process Replacing Test")
    {
        float phase = 0.0f;
        const auto phaseIncrement = juce::MathConstants<float>::twoPi * testFreq / float (_sampleRate);
        auto refOsc = [&phase, phaseIncrement] (float input) mutable
        {
            const auto y = polygonalCore (phase, polygonOrder, polygonTeeth);

            phase += phaseIncrement;
            while (phase >= juce::MathConstants<float>::twoPi)
                phase -= juce::MathConstants<float>::twoPi;

            return y + input;
        };

        chowdsp::experimental::PolygonalOscillator<float> testOsc;
        testOsc.prepare ({ _sampleRate, (juce::uint32) _blockSize, 1 });
        testOsc.setFrequency (testFreq);
        testOsc.setOrder (polygonOrder);
        testOsc.setTeeth (polygonTeeth);

        chowdsp::Buffer<float> testBuffer (1, 21);
        juce::FloatVectorOperations::fill (testBuffer.getWritePointer (0), 1.0f, 21);
        testOsc.processBlock (testBuffer);

        for (int i = 0; i < 19; ++i)
        {
            auto expOut = refOsc (1.0f);
            auto actualOut = testBuffer.getReadPointer (0)[i];
            REQUIRE_MESSAGE (actualOut == Catch::Approx (expOut).margin (0.01f), "Generated sample is incorrect!");
        }
    }

    SECTION ("Zero Hz Test")
    {
        chowdsp::experimental::PolygonalOscillator<float> testOsc;
        testOsc.prepare ({ _sampleRate, (juce::uint32) _blockSize, 1 });
        testOsc.setFrequency (0.0f);

        for (int i = 0; i < 10; ++i)
            REQUIRE_MESSAGE (juce::approximatelyEqual (testOsc.processSample(), 0.0f), "Zero Hz output is non-zero!");
    }
}
