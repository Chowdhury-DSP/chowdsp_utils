#include <CatchUtils.h>
#include <chowdsp_waveshapers/chowdsp_waveshapers.h>

TEST_CASE ("West Coast Folder Test", "[dsp][waveshapers]")
{
    SECTION ("Process Test")
    {
        chowdsp::WestCoastWavefolder<float> folder;
        folder.prepare (1);

        static constexpr size_t N = 20;
        std::array<float, N> inputVals {};
        std::array<float, N> expVals { 0.0f, -2.37976718f, 3.30801344f, 3.30801296f, -3.39300186E-7f, -3.30801439f, -3.30801249f, -2.05490551E-7f, 3.30801439f, 3.30801105f, 0.00000159522051f, -3.30801916f, -3.30800867f, -0.00000129507316f, 3.3080163f, 3.30801058f, -0.00000283613736f, -3.30801249f, -3.30801105f, 5.12237932E-8f };
        std::array<float, N> actualVals {};

        for (size_t i = 0; i < N; ++i)
        {
            inputVals[i] = 10.0f * std::sin (juce::MathConstants<float>::twoPi * (float) i * 8000.0f / 48000.0f);
            actualVals[i] = folder.processSample (inputVals[i]);

            REQUIRE (actualVals[i] == Catch::Approx (expVals[i]).margin (1.0e-6));
        }
    }
}
