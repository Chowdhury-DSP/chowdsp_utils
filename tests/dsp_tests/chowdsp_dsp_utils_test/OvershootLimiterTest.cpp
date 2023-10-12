#include <CatchUtils.h>
#include <chowdsp_dsp_utils/chowdsp_dsp_utils.h>

TEST_CASE ("Overshoot Limiter Test", "[dsp][misc]")
{
    constexpr double fs = 48000.0;
    constexpr int N = 4800;

    chowdsp::OvershootLimiter<float> limiter { 64 };
    limiter.prepare ({ fs, (uint32_t) N, 1 });
    REQUIRE (limiter.getLatencySamples() == 64);

    auto signal = test_utils::makeSineWave (100.0f, (float) fs, N);

    SECTION ("Ceiling == 1")
    {
        chowdsp::BufferMath::applyGain (signal, 1.1f);

        limiter.setCeiling (1.0f);
        limiter.processBlock (signal);
        REQUIRE (chowdsp::BufferMath::getMagnitude (signal) <= 1.0f);
    }

    SECTION ("Ceiling == 0.5")
    {
        limiter.setCeiling (0.5f);
        limiter.processBlock (signal);
        REQUIRE (chowdsp::BufferMath::getMagnitude (signal) <= 0.5f);
    }
}
