#include <TimedUnitTest.h>
#include <chowdsp_dsp_utils/chowdsp_dsp_utils.h>

namespace
{
constexpr int N = 1000;
constexpr float maxErr = 1.0e-1f;
} // namespace

class ADAAHardClipperTest : public TimedUnitTest
{
public:
    ADAAHardClipperTest() : TimedUnitTest ("ADAA Hard Clipper Test") {}

    void processTest (bool inPlace, bool isBypassed = false)
    {
        chowdsp::ADAAHardClipper<float> clipper;
        clipper.prepare (1);

        juce::AudioBuffer<float> testBuffer (1, N);
        float expYs[N];
        for (int i = 0; i < N; ++i)
        {
            const auto testX = 2.5f * std::sin (juce::MathConstants<float>::twoPi * (float) i * 500.0f / 48000.0f);
            testBuffer.setSample (0, i, testX);

            if (isBypassed)
                expYs[i] = testX;
            else
                expYs[i] = juce::jlimit (-1.0f, 1.0f, testX);
        }

        juce::AudioBuffer<float> outBuffer (1, N);
        if (inPlace)
        {
            auto&& testBlock = juce::dsp::AudioBlock<float> { testBuffer };
            auto&& context = juce::dsp::ProcessContextReplacing<float> { testBlock };
            context.isBypassed = isBypassed;
            clipper.process (context);
        }
        else
        {
            auto&& testBlock = juce::dsp::AudioBlock<float> { testBuffer };
            auto&& outBlock = juce::dsp::AudioBlock<float> { outBuffer };

            auto&& context = juce::dsp::ProcessContextNonReplacing<float> { testBlock, outBlock };
            context.isBypassed = isBypassed;
            clipper.process (context);
        }

        for (int i = 2; i < N; ++i)
        {
            float actualY;
            if (inPlace)
                actualY = testBuffer.getSample (0, i);
            else
                actualY = outBuffer.getSample (0, i);

            expectWithinAbsoluteError (actualY, expYs[i - 1], maxErr, "Hard Clipper value is incorrect! " + juce::String (testBuffer.getSample (0, i)));
        }
    }

    void runTestTimed() override
    {
        beginTest ("Process Test");
        processTest (true);
        processTest (false);

        beginTest ("Bypassed Test");
        processTest (true, true);
        processTest (false, true);
    }
};

static ADAAHardClipperTest adaaHardClipperTest;
