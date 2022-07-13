#include <CatchUtils.h>
#include <chowdsp_waveshapers/chowdsp_waveshapers.h>

namespace Constants
{
constexpr int N = 1000;
constexpr float maxErr = 1.0e-1f;
} // namespace Constants

template <int degree>
void processTest (chowdsp::LookupTableCache& lutCache)
{
    chowdsp::ADAASoftClipper<float, degree> clipper { &lutCache };
    clipper.prepare (1);

    chowdsp::Buffer<float> testBuffer (1, Constants::N);
    float expYs[Constants::N];
    for (int i = 0; i < Constants::N; ++i)
    {
        const auto testX = 2.5f * std::sin (juce::MathConstants<float>::twoPi * (float) i * 500.0f / 48000.0f);
        testBuffer.getWritePointer (0)[i] = testX;
        expYs[i] = chowdsp::SoftClipper<degree, float>::processSample (testX);
    }

    clipper.processBlock (testBuffer);

    for (int i = 1; i < Constants::N; ++i)
    {
        float actualY = testBuffer.getReadPointer (0)[i];
        REQUIRE_MESSAGE (actualY == Approx (expYs[i - 1]).margin (Constants::maxErr), "Soft Clipper value is incorrect!");
    }
}

TEST_CASE ("ADAA Soft Clipper Test")
{
    chowdsp::LookupTableCache lutCache;

    processTest<3> (lutCache);
    processTest<3> (lutCache);
    processTest<5> (lutCache);
    processTest<9> (lutCache);

    lutCache.clearCache();
}
