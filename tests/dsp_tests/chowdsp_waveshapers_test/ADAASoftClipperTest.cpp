#include <CatchUtils.h>
#include <chowdsp_waveshapers/chowdsp_waveshapers.h>

namespace Constants
{
constexpr int N = 1000;
constexpr float maxErr = 1.0e-1f;
} // namespace Constants

template <int degree, chowdsp::ADAAWaveshaperMode mode>
void processTest (chowdsp::LookupTableCache& lutCache, bool processBuffer = true)
{
    chowdsp::ADAASoftClipper<float, degree, mode> clipper { &lutCache };
    clipper.prepare (1);

    chowdsp::Buffer<float> testBuffer (1, Constants::N);
    float expYs[Constants::N];
    for (int i = 0; i < Constants::N; ++i)
    {
        const auto testX = 2.5f * std::sin (juce::MathConstants<float>::twoPi * (float) i * 500.0f / 48000.0f);
        testBuffer.getWritePointer (0)[i] = testX;
        expYs[i] = chowdsp::SoftClipper<degree, float>::processSample (testX);
    }

    if (processBuffer)
    {
        clipper.processBlock (testBuffer);
    }
    else
    {
        for (auto [channel, channelData] : chowdsp::buffer_iters::channels (testBuffer))
        {
            for (auto& x : channelData)
                x = clipper.processSample (x, channel);
        }
    }

    for (int i = 1; i < Constants::N; ++i)
    {
        float actualY = testBuffer.getReadPointer (0)[i];
        REQUIRE_MESSAGE (actualY == Catch::Approx (expYs[i - 1]).margin (Constants::maxErr), "Soft Clipper value is incorrect!");
    }
}

TEST_CASE ("ADAA Soft Clipper Test", "[dsp][waveshapers]")
{
    chowdsp::LookupTableCache lutCache;

    using Mode = chowdsp::ADAAWaveshaperMode;
    processTest<3, Mode::Direct> (lutCache, false);
    lutCache.clearCache();
    processTest<3, Mode::MinusX> (lutCache);
    processTest<5, Mode::Direct> (lutCache);
    processTest<9, Mode::MinusX> (lutCache, false);

    lutCache.clearCache();
}
