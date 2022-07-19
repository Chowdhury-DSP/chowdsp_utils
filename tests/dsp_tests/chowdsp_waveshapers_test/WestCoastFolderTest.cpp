#include <CatchUtils.h>
#include <chowdsp_waveshapers/chowdsp_waveshapers.h>

TEST_CASE ("West Coast Folder Test")
{
    SECTION ("Process Test")
    {
        chowdsp::WestCoastWavefolder<float> folder;
        folder.prepare (1);

        static constexpr size_t N = 20;
        std::array<float, N> inputVals {};
        std::array<float, N> expVals { 0.0f, 0.506984353f, 0.421261728f, 0.421261728f, -0.0000000743737019f, -0.421262234f, -0.421262264f, 0.000000218332659f, 0.42126143f, 0.42126146f, -0.000000153132461f, -0.421262503f, -0.421262532f, 0.000000506250728f, 0.42126143f, 0.421261519f, -0.000000557832607f, -0.4212614f, -0.42126146f, 0.0000000511191196f };
        std::array<float, N> actualVals {};

        for (size_t i = 0; i < N; ++i)
        {
            inputVals[i] = 10.0f * std::sin (juce::MathConstants<float>::twoPi * (float) i * 8000.0f / 48000.0f);
            actualVals[i] = folder.processSample (inputVals[i]);

            REQUIRE (actualVals[i] == Approx (expVals[i]).margin (1.0e-6));
        }
    }
}
