#include <CatchUtils.h>
#include <chowdsp_waveshapers/chowdsp_waveshapers.h>
#include <iostream>

TEST_CASE ("Wave Multiplier Test", "[dsp][waveshapers]")
{
    SECTION ("Process Test")
    {
        chowdsp::WaveMultiplier<float> folder;
        folder.prepare (1);

        static constexpr size_t N = 20;
        std::array<float, N> inputVals {};
        std::array<float, N> expVals { 0.0f, 0.00000270857777f, -0.0291773807f, 1.16696322f, -3.44325686f, 5.96103f, -7.11124039f, 6.43918467f, -4.06395102f, 1.56404686f, -0.00069783628f, 0.190425128f, -0.0000017228499f, -0.188422918f, -0.188428909f, 0.00000622259586f, 0.188418567f, 0.18843393f, -0.0000069831076f, -0.188420907f };
        std::array<float, N> actualVals {};

        for (size_t i = 0; i < N; ++i)
        {
            inputVals[i] = 10.0f * std::sin (juce::MathConstants<float>::twoPi * (float) i * 8000.0f / 48000.0f);
            actualVals[i] = folder.processSample (inputVals[i]);

            REQUIRE (actualVals[i] == Catch::Approx (expVals[i]).margin (1.0e-6));
        }
    }
}
