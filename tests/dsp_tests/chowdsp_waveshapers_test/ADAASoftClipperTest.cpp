#include <TimedUnitTest.h>
#include <chowdsp_waveshapers/chowdsp_waveshapers.h>

namespace Constants
{
constexpr int N = 1000;
constexpr float maxErr = 1.0e-1f;
} // namespace Constants

class ADAASoftClipperTest : public TimedUnitTest
{
public:
    ADAASoftClipperTest() : TimedUnitTest ("ADAA Soft Clipper Test") {}

    void shutdown() override
    {
        lutCache->clearCache();
    }

    template <int degree>
    void processTest (bool inPlace, bool isBypassed = false)
    {
        chowdsp::ADAASoftClipper<float, degree> clipper { &lutCache.get() };
        clipper.prepare (1);

        juce::AudioBuffer<float> testBuffer (1, Constants::N);
        float expYs[Constants::N];
        for (int i = 0; i < Constants::N; ++i)
        {
            const auto testX = 2.5f * std::sin (juce::MathConstants<float>::twoPi * (float) i * 500.0f / 48000.0f);
            testBuffer.setSample (0, i, testX);

            if (isBypassed)
                expYs[i] = testX;
            else
                expYs[i] = chowdsp::SoftClipper<degree, float>::processSample (testX);
        }

        juce::AudioBuffer<float> outBuffer (1, Constants::N);
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

        for (int i = 1; i < Constants::N; ++i)
        {
            float actualY;
            if (inPlace)
                actualY = testBuffer.getSample (0, i);
            else
                actualY = outBuffer.getSample (0, i);

            expectWithinAbsoluteError (actualY, expYs[i - 1], Constants::maxErr, "Soft Clipper value is incorrect! " + juce::String (testBuffer.getSample (0, i)));
        }
    }

    void runTestTimed() override
    {
        beginTest ("Process Test");
        processTest<3> (true);
        processTest<3> (false); // helpful for testing the LookupTableCache
        processTest<5> (true);
        processTest<9> (false);
    }

private:
    chowdsp::SharedLookupTableCache lutCache;
};

static ADAASoftClipperTest adaaSoftClipperTest;
