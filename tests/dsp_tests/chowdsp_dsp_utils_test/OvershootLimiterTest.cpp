#include <CatchUtils.h>
#include <chowdsp_dsp_utils/chowdsp_dsp_utils.h>

namespace
{
static constexpr double fs = 48000.0;
static constexpr int N = 4800;
static constexpr int lookahead = 64;

static void processInBlocks (chowdsp::OvershootLimiter<float>& limiter, chowdsp::BufferView<float> buffer, int blockSize)
{
    for (int start = 0; start < buffer.getNumSamples(); start += blockSize)
    {
        const auto numToProcess = juce::jmin (blockSize, buffer.getNumSamples() - start);
        limiter.processBlock (chowdsp::BufferView<float> { buffer, start, numToProcess });
    }
}
} // namespace

TEST_CASE ("Overshoot Limiter Test", "[dsp][misc]")
{
    chowdsp::OvershootLimiter<float> limiter { lookahead };
    limiter.prepare ({ fs, (uint32_t) N, 1 });
    REQUIRE (limiter.getLatencySamples() == lookahead);

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

    SECTION ("Peak at the start of the block")
    {
        signal.clear();
        signal.getWritePointer (0)[0] = 2.0f;

        limiter.setCeiling (1.0f);
        limiter.processBlock (signal);
        REQUIRE (chowdsp::BufferMath::getMagnitude (signal) <= 1.0f);
    }

    SECTION ("Sine wave starting at its peak")
    {
        // cosine wave: the first sample is the maximum
        auto* x = signal.getWritePointer (0);
        for (int n = 0; n < N; ++n)
            x[n] = 1.5f * std::cos (juce::MathConstants<float>::twoPi * 100.0f * (float) n / (float) fs);

        limiter.setCeiling (1.0f);
        limiter.processBlock (signal);
        REQUIRE (chowdsp::BufferMath::getMagnitude (signal) <= 1.0f);
    }

    SECTION ("Latency")
    {
        constexpr int blockSize = 256;
        constexpr int impulseIndex = 100;

        limiter.prepare ({ fs, (uint32_t) blockSize, 1 });
        limiter.setCeiling (1.0f);

        // below the ceiling, so the limiter should apply no gain reduction at all,
        // and the output should just be a delayed version of the input.
        signal.clear();
        signal.getWritePointer (0)[impulseIndex] = 0.5f;

        processInBlocks (limiter, signal, blockSize);

        const auto* y = signal.getReadPointer (0);
        const auto expectedIndex = impulseIndex + limiter.getLatencySamples();
        for (int n = 0; n < N; ++n)
        {
            const auto expected = n == expectedIndex ? 0.5f : 0.0f;
            INFO ("Sample index: " << n);
            REQUIRE (y[n] == Catch::Approx { expected }.margin (1.0e-6f));
        }
    }

    SECTION ("Overs")
    {
        struct TestConfig
        {
            float ceiling;
            float peakGain;
            int blockSize;
            int peakIndex;
        };

        const auto config = GENERATE (TestConfig { 1.0f, 1.1f, N, 0 },
                                      TestConfig { 1.0f, 2.0f, N, 0 },
                                      TestConfig { 0.5f, 1.1f, 512, 0 },
                                      TestConfig { 1.0f, 1.1f, 512, 512 }, // peak at the start of the 2nd block
                                      TestConfig { 1.0f, 1.1f, 512, 1024 }, // peak at the start of the 3rd block
                                      TestConfig { 1.0f, 2.0f, 512, 1024 + 3 },
                                      TestConfig { 1.0f, 1.1f, 128, 640 },
                                      TestConfig { 0.5f, 3.0f, 100, 1000 });

        INFO ("Ceiling: " << config.ceiling << ", Peak Gain: " << config.peakGain << ", Block Size: " << config.blockSize << ", Peak Index: " << config.peakIndex);

        limiter.prepare ({ fs, (uint32_t) config.blockSize, 1 });
        limiter.setCeiling (config.ceiling);

        // quiet signal, with a single loud sample at the peak index
        chowdsp::BufferMath::applyGain (signal, 0.25f * config.ceiling);
        signal.getWritePointer (0)[config.peakIndex] = config.peakGain * config.ceiling;

        processInBlocks (limiter, signal, config.blockSize);

        const auto* y = signal.getReadPointer (0);
        for (int n = 0; n < N; ++n)
        {
            INFO ("Sample index: " << n);
            REQUIRE (std::abs (y[n]) <= config.ceiling);
        }
    }
}
