#include <CatchUtils.h>
#include <chowdsp_filters/chowdsp_filters.h>

namespace Constants
{
constexpr float fs = 48000.0f;
constexpr float lowGain = 0.5f;
constexpr float highGain = 2.0f;
constexpr float fc = 1000.0f;
constexpr float maxError = 0.1f;
} // namespace Constants

/** Unit tests for chowdsp::ShelfFilter. Checks for correct gain at:
 *   - All frequencies when high gain == low gain
 *   - Low frequencies cut by -6 dB
 *   - High frequencies boosted by +6 dB
 *   - Transition frequency, stable at +0 dB
 */
TEMPLATE_TEST_CASE ("Shelf Filter Test", "[dsp][filters][simd]", float, xsimd::batch<float>)
{
    using T = TestType;
    using NumericType = chowdsp::SampleTypeHelpers::NumericType<T>;

    SECTION ("Plain Gain Test")
    {
        chowdsp::ShelfFilter<T> shelfFilter;
        shelfFilter.reset();
        shelfFilter.calcCoefs ((T) 2, (T) 2, (T) Constants::fc, Constants::fs);

        auto buffer = test_utils::makeNoise<T> ((int) Constants::fs);
        const auto refMag = chowdsp::BufferMath::getRMSLevel (buffer, 0);

        shelfFilter.processBlock (buffer);

        const auto mag = chowdsp::BufferMath::getRMSLevel (buffer, 0);

        REQUIRE_MESSAGE (mag / refMag == SIMDApprox<T> ((T) 2.0f).margin (1.0e-6f), "Incorrect behavior when filter reduces to a simple gain.");
    }

    SECTION ("Boost/Cut Test")
    {
        chowdsp::ShelfFilter<T> shelfFilter;
        shelfFilter.calcCoefs ((T) Constants::lowGain, (T) Constants::highGain, (T) Constants::fc, Constants::fs);

        auto testFrequency = [=, &shelfFilter] (float freq, float expGain, const std::string& message)
        {
            auto buffer = test_utils::makeSineWave<T> (freq, Constants::fs, (NumericType) 1);

            shelfFilter.reset();
            shelfFilter.processBlock (buffer);

            auto mag = chowdsp::BufferMath::getMagnitude (buffer, 0, buffer.getNumSamples());
            REQUIRE_MESSAGE (mag == SIMDApprox<T> ((T) expGain).margin (Constants::maxError), message);
        };

        testFrequency (10.0f, Constants::lowGain, "Incorrect gain at low frequencies.");
        testFrequency (Constants::fc, 1.0f, "Incorrect gain at transition frequency.");
        testFrequency (20000.0f, Constants::highGain, "Incorrect gain at high frequencies.");
    }
}
