#include <CatchUtils.h>
#include <chowdsp_math/chowdsp_math.h>

namespace da = chowdsp::DecibelsApprox;

TEMPLATE_TEST_CASE ("Decibels Approx Test", "[dsp][math][simd]", float, double, xsimd::batch<float>, xsimd::batch<double>)
{
    using FloatType = TestType;
    using NumericType = chowdsp::SampleTypeHelpers::NumericType<FloatType>;

    SECTION ("Decibels -> Gain")
    {
        for (auto x = (NumericType) -110; x < (NumericType) 24; x += (NumericType) 0.1)
        {
            const auto ref = juce::Decibels::decibelsToGain (x);
            REQUIRE (da::decibelsToGain (x) == SIMDApprox<FloatType> { ref }.margin ((NumericType) 0.01));
        }
    }

    SECTION ("Gain -> Decibels")
    {
        for (auto x = (NumericType) -110; x < (NumericType) 24; x += (NumericType) 0.1)
        {
            const auto gain = juce::Decibels::decibelsToGain (x, (NumericType) -200);
            const auto ref = juce::Decibels::gainToDecibels (gain);
            REQUIRE (da::gainToDecibels (gain) == SIMDApprox<FloatType> { ref }.margin ((NumericType) 0.05));
        }
    }
}
