#include <CatchUtils.h>
#include <chowdsp_math/chowdsp_math.h>

TEST_CASE ("Constexpr Ratio Test", "[dsp][math]")
{
    SECTION ("Ratio Test")
    {
        static_assert (juce::exactlyEqual (chowdsp::Ratio<33, 100>::value<float>, 0.33f));
        static_assert (juce::exactlyEqual (chowdsp::Ratio<100, 1>::value<double>, 100.0));
        static_assert (juce::exactlyEqual (chowdsp::Ratio<25, 50>::value<float>, 0.5f));
        static_assert (juce::exactlyEqual (chowdsp::Ratio<12, 10>::value<float>, 1.2f));
    }

    SECTION ("Scientific Ratio Test")
    {
        static_assert (juce::exactlyEqual (chowdsp::ScientificRatio<33, 0>::value<float>, 33.0f));
        static_assert (juce::exactlyEqual (chowdsp::ScientificRatio<12, -5>::value<float>, 12.0e-5f));
        static_assert (juce::exactlyEqual (chowdsp::ScientificRatio<42, 10>::value<double>, 42.0e10));
    }
}
