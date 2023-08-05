#include <CatchUtils.h>
#include <chowdsp_modal_dsp/chowdsp_modal_dsp.h>

namespace
{
constexpr float fs = 44100.0f;
constexpr float modeDecay = 0.01f;
constexpr float modeAmp = 0.1f;
} // namespace

template <typename T>
static std::vector<T> getTestVector (const chowdsp::Buffer<float>& buffer)
{
    using NumericType = chowdsp::SampleTypeHelpers::NumericType<T>;
    std::vector<T> vector ((size_t) buffer.getNumSamples(), (T) 0);
    for (int i = 0; i < buffer.getNumSamples(); ++i)
        vector[(size_t) i] = (T) (NumericType) buffer.getReadPointer (0)[i];

    return vector;
}

template <typename T>
static typename std::enable_if_t<std::is_floating_point_v<T>, T>
    getMagnitude (const std::vector<T>& buffer, int start = 0, int num = -1)
{
    num = num >= 0 ? num : (int) buffer.size();

    auto max = (T) 0;
    for (int i = start; i < start + num; ++i)
        max = juce::jmax (max, buffer[(size_t) i]);

    return max;
}

template <typename T>
static typename std::enable_if_t<chowdsp::SampleTypeHelpers::IsSIMDRegister<T>, chowdsp::SampleTypeHelpers::NumericType<T>>
    getMagnitude (const std::vector<T>& buffer, int start = 0, int num = -1)
{
    num = num >= 0 ? num : (int) buffer.size();

    auto max = (T) 0;
    for (int i = start; i < start + num; ++i)
        max = xsimd::max (max, buffer[(size_t) i]);

    return max.get (0);
}

/** Unit tests for chowdsp::ModalFilter. Tests include:
 *   - Check that filter resonates at center frequency
 *   - Check that filter damps frequencies other than center frequency
 *   - Check that filter has correct decay time
 */
TEMPLATE_TEST_CASE ("Modal Filter Test", "[dsp][modal][simd]", float, double, xsimd::batch<float>, xsimd::batch<double>)
{
    using T = TestType;
    using NumericType = chowdsp::SampleTypeHelpers::NumericType<T>;

    SECTION ("Resonant Frequency Sine Test")
    {
        constexpr float testFreq = 100.0f;
        auto buffer = getTestVector<T> (test_utils::makeSineWave (testFreq, fs, 1.0f));
        auto refMag = juce::Decibels::gainToDecibels (getMagnitude (buffer));

        chowdsp::ModalFilter<T> filter;
        filter.prepare ((NumericType) fs);
        filter.setAmp ((T) modeAmp);
        filter.setDecay ((T) modeDecay);
        filter.setFreq ((T) testFreq);
        filter.processBlock (buffer.data(), (int) buffer.size());

        auto mag = juce::Decibels::gainToDecibels (getMagnitude (buffer));
        REQUIRE_MESSAGE ((mag - refMag) > (NumericType) 6.0f, "Modal filter is not resonating at correct frequency.");
    }

    SECTION ("Damped Frequency Sine Test")
    {
        constexpr float testFreq1 = 100.0f;
        constexpr float testFreq2 = 10000.0f;
        auto buffer = getTestVector<T> (test_utils::makeSineWave (testFreq1, fs, 1.0f));
        auto refMag = juce::Decibels::gainToDecibels (getMagnitude (buffer));

        chowdsp::ModalFilter<T> filter;
        filter.prepare ((NumericType) fs);
        filter.setAmp ((T) modeAmp);
        filter.setDecay ((T) modeDecay);
        filter.setFreq ((T) testFreq2);
        filter.processBlock (buffer.data(), (int) buffer.size());

        if constexpr (std::is_floating_point_v<T>)
            REQUIRE_MESSAGE (juce::exactlyEqual (filter.getFreq(), (NumericType) testFreq2), "Modal filter frequency is incorrect");
        else
            REQUIRE_MESSAGE (juce::exactlyEqual (filter.getFreq().get (0), (NumericType) testFreq2), "Modal filter frequency is incorrect");

        auto mag = juce::Decibels::gainToDecibels (getMagnitude (buffer));
        REQUIRE_MESSAGE ((mag - refMag) < (NumericType) -24.0f, "Modal filter is resonating at an incorrect frequency.");
    }

    SECTION ("Decay Time Test")
    {
        auto buffer = getTestVector<T> (test_utils::makeImpulse (1.0f, fs, 1.0f));

        chowdsp::ModalFilter<T> filter;
        filter.prepare ((NumericType) fs);
        filter.setAmp ((T) 1.0f);
        filter.setDecay ((T) 0.5f);
        filter.setFreq ((T) 100.0f);
        filter.processBlock (buffer.data(), (int) buffer.size());

        auto mag = juce::Decibels::gainToDecibels (getMagnitude (buffer, int (fs * 0.5f), int (fs * 0.1f)));
        REQUIRE_MESSAGE (mag == Catch::Approx ((NumericType) -60.0f).margin ((NumericType) 1.0f), "Incorrect decay time.");
    }

    SECTION ("Initial Phase Test")
    {
        static constexpr auto initialPhase = (NumericType) 0.5f;

        chowdsp::ModalFilter<T> filter;
        filter.prepare ((NumericType) fs);
        filter.setAmp ((T) 1.0f, initialPhase);
        filter.setDecay ((T) 0.5f);
        filter.setFreq ((T) 100.0f);

        const auto actual = filter.processSample ((T) 1);
        const auto expected = std::sin (initialPhase);

        JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wfloat-equal")
        REQUIRE_MESSAGE (chowdsp::SIMDUtils::all (actual == expected), "Incorrect initial phase!");
        JUCE_END_IGNORE_WARNINGS_GCC_LIKE
    }
}
