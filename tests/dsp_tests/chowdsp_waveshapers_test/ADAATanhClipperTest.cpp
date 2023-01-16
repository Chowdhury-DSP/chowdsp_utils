#include <CatchUtils.h>
#include <chowdsp_waveshapers/chowdsp_waveshapers.h>

namespace Constants
{
constexpr int N = 1000;
constexpr float maxErr = 1.0e-2f;
} // namespace Constants

static void processTest (bool isBypassed = false)
{
    chowdsp::ADAATanhClipper<float> clipper;
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
            expYs[i] = std::tanh (testX);
    }

    if (isBypassed)
        clipper.processBlockBypassed (testBuffer);
    else
        clipper.processBlock (testBuffer);

    for (int i = 2; i < Constants::N - 1; ++i)
    {
        float actualY = testBuffer.getReadPointer (0)[i];
        REQUIRE_MESSAGE (actualY == Catch::Approx (expYs[i - 1]).margin (Constants::maxErr), "Tanh Clipper value is incorrect!");
    }
}

TEST_CASE ("ADAA Tanh Clipper Test", "[dsp][waveshapers]")
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
