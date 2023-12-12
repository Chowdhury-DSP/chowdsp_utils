#include <CatchUtils.h>
#include <chowdsp_filters/chowdsp_filters.h>

namespace Constants
{
constexpr float fs = 48000.0f;
constexpr float fc = 1000.0f;
} // namespace Constants

template <typename T, typename Filter>
void testFilter (Filter& filt, std::vector<float> freqs, std::vector<float> mags, std::vector<float> errs, const std::vector<std::string>& messages)
{
    auto testFrequency = [&filt] (float freq, float expGain, float err, const std::string& message)
    {
        auto buffer = test_utils::makeSineWave<T> (freq, Constants::fs, 1.0f);

        filt.prepare (1);
        filt.reset();
        filt.processBlock (buffer);

        const auto halfSamples = buffer.getNumSamples() / 2;
        auto mag = chowdsp::SIMDUtils::gainToDecibels (chowdsp::BufferMath::getMagnitude (buffer, halfSamples, halfSamples));
        REQUIRE_MESSAGE (mag == SIMDApprox<T> ((T) expGain).margin (err), message);
    };

    for (size_t i = 0; i < freqs.size(); ++i)
        testFrequency (freqs[i], mags[i], errs[i], "Incorrect gain at " + messages[i] + " frequency.");
}

TEMPLATE_TEST_CASE ("Butterworth Filter Test", "[dsp][filters][anti-aliasing]", float)
{
    using T = TestType;
    using FilterType = chowdsp::ButterworthFilterType;

    SECTION ("LPF Even Test")
    {
        chowdsp::ButterworthFilter<4, FilterType::Lowpass, T> filter;
        filter.calcCoefs (Constants::fc, (T) 1 / juce::MathConstants<T>::sqrt2, Constants::fs);

        testFilter<T> (filter,
                       { 100.0f, Constants::fc, 4 * Constants::fc },
                       { 0.0f, -3.0f, -48.0f },
                       { 0.005f, 0.01f, 0.5f },
                       { "passband", "cutoff", "stopband" });
    }

    SECTION ("LPF Odd Test")
    {
        chowdsp::ButterworthFilter<3, FilterType::Lowpass, T> filter;
        filter.calcCoefs (Constants::fc, (T) 1 / juce::MathConstants<T>::sqrt2, Constants::fs);

        testFilter<T> (filter,
                       { 100.0f, Constants::fc, 4 * Constants::fc },
                       { 0.0f, -3.0f, -36.0f },
                       { 0.005f, 0.01f, 0.75f },
                       { "passband", "cutoff", "stopband" });
    }

    SECTION ("HPF Even Test")
    {
        chowdsp::ButterworthFilter<4, FilterType::Highpass, T> filter;
        filter.calcCoefs (Constants::fc, (T) 1 / juce::MathConstants<T>::sqrt2, Constants::fs);

        testFilter<T> (filter,
                       { 10000.0f, Constants::fc, 0.25f * Constants::fc },
                       { 0.0f, -3.0f, -48.0f },
                       { 0.01f, 0.01f, 0.5f },
                       { "passband", "cutoff", "stopband" });
    }

    SECTION ("HPF Odd Test")
    {
        chowdsp::ButterworthFilter<3, FilterType::Highpass, T> filter;
        filter.calcCoefs (Constants::fc, (T) 1 / juce::MathConstants<T>::sqrt2, Constants::fs);

        testFilter<T> (filter,
                       { 10000.0f, Constants::fc, 0.25f * Constants::fc },
                       { 0.0f, -3.0f, -36.0f },
                       { 0.025f, 0.01f, 0.5f },
                       { "passband", "cutoff", "stopband" });
    }
}
