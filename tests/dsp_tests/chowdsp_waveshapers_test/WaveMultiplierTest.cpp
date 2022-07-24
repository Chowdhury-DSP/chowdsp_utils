#include <CatchUtils.h>
#include <chowdsp_waveshapers/chowdsp_waveshapers.h>
#include <iostream>

TEST_CASE ("Wave Multiplier Test")
{
    SECTION ("Process Test")
    {
        chowdsp::WaveMultiplier<float> folder;
        folder.prepare (1);

        static constexpr size_t N = 20;
        std::array<float, N> inputVals {};
        std::array<float, N> expVals { 0.0f, 0.000171947f, 0.0052776f, -0.00291242f, -0.0234547f, -0.0472894f, -0.0515148f, -0.0347016f, -0.00229279f, 0.0498638f, 0.0933643f, 0.0807403f, -3.65864e-08f, -0.0830418f, -0.0830417f, 7.77615e-10f, 0.0830417f, 0.0830417f, -7.92645e-08f, -0.0830418f };
        std::array<float, N> actualVals {};

        for (size_t i = 0; i < N; ++i)
        {
            inputVals[i] = 10.0f * std::sin (juce::MathConstants<float>::twoPi * (float) i * 8000.0f / 48000.0f);
            actualVals[i] = folder.processSample (inputVals[i]);

            REQUIRE (actualVals[i] == Approx (expVals[i]).margin (1.0e-6));
        }
    }
}
