#include <CatchUtils.h>
#include <chowdsp_waveshapers/chowdsp_waveshapers.h>

namespace Constants
{
constexpr int N = 1000;
constexpr float maxErr = 1.0e-1f;
} // namespace Constants

static void processTest (bool isBypassed = false)
{
    chowdsp::ADAAHardClipper<float> clipper;
    clipper.prepare (1);

    chowdsp::Buffer<float> testBuffer (1, Constants::N);
    float expYs[Constants::N];
    for (int i = 0; i < Constants::N; ++i)
    {
        const auto testX = 2.5f * std::sin (juce::MathConstants<float>::twoPi * (float) i * 500.0f / 48000.0f);
        testBuffer.getWritePointer (0)[i] = testX;

        if (isBypassed)
            expYs[i] = testX;
        else
            expYs[i] = juce::jlimit (-1.0f, 1.0f, testX);
    }

    if (isBypassed)
        clipper.processBlockBypassed (testBuffer);
    else
        clipper.processBlock (testBuffer);

    for (int i = 1; i < Constants::N; ++i)
    {
        float actualY = testBuffer.getReadPointer (0)[i];
        REQUIRE_MESSAGE (actualY == Approx (expYs[i - 1]).margin (Constants::maxErr), "Hard Clipper value is incorrect!");
    }
}

TEST_CASE ("ADAA Hard Clipper Test")
{
    SECTION ("Process Test")
    {
        processTest();
        processTest();
    }

    SECTION ("Bypassed Test")
    {
        processTest (true);
        processTest (true);
    }
}
