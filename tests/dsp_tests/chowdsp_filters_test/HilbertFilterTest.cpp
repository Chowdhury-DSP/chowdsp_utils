#include <CatchUtils.h>
#include <chowdsp_filters/chowdsp_filters.h>
#include <chowdsp_dsp_utils/chowdsp_dsp_utils.h>

namespace Constants
{
constexpr auto sampleRate = 48000.0;
constexpr auto sineFreq = 1000.0;
constexpr int N = 140;
} // namespace Constants

TEST_CASE ("Hilbert Filter Test", "[dsp][filters]")
{
    SECTION ("Phase Shift Test")
    {
        using namespace Constants;

        constexpr auto phaseShiftDelaySamples = sampleRate / sineFreq * 0.25;

        chowdsp::HilbertFilter<double> hilbert;
        hilbert.reset();

        chowdsp::DelayLine<double> shifter { 1 << 18 };
        shifter.prepare ({ sampleRate, 128, 1 });
        shifter.setDelay (phaseShiftDelaySamples);

        for (int n = 0; n < N; ++n)
        {
            const auto inPhase = juce::MathConstants<double>::twoPi * (double) n * sineFreq / sampleRate;
            const auto inSample = std::sin (inPhase);

            auto [out, out90] = hilbert.process (inSample);

            shifter.pushSample (0, out90);
            auto outCorr = shifter.popSample (0);

            //            std::cout << out << ", " << out90 << ", " << outCorr << std::endl;

            if (n > 64)
            {
                const auto error = std::abs (out - outCorr) / (1.0 + std::abs (out));
                REQUIRE_MESSAGE (error < 0.1, "Phase shift error is too high!");
            }
        }
    }
}
