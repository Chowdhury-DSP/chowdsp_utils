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

        filt.reset();
        filt.processBlock (buffer);

        const auto halfSamples = buffer.getNumSamples() / 2;
        auto mag = chowdsp::SIMDUtils::gainToDecibels (chowdsp::BufferMath::getMagnitude (buffer, halfSamples, halfSamples));

        if (err < 0.0f)
            REQUIRE_MESSAGE (mag <= expGain, message);
        else
            REQUIRE_MESSAGE (mag == SIMDApprox<T> ((T) expGain).margin (err), message);
    };

    for (size_t i = 0; i < freqs.size(); ++i)
        testFrequency (freqs[i], mags[i], errs[i], "Incorrect gain at " + messages[i] + " frequency.");
}

TEMPLATE_TEST_CASE ("Chebyshev II Filter Test", "[dsp][filters][anti-aliasing]", float)
{
    using T = TestType;
    using FilterType = chowdsp::ChebyshevFilterType;

    SECTION ("LPF Test")
    {
        { // Chebyshev cutoff frequency
            chowdsp::ChebyshevIIFilter<8, FilterType::Lowpass, 60, false, T> filter;
            filter.calcCoefs (Constants::fc, (T) 1 / juce::MathConstants<T>::sqrt2, Constants::fs);

            testFilter<T> (filter,
                           { 100.0f, Constants::fc, 4 * Constants::fc },
                           { 0.0f, -60.0f, -60.0f },
                           { 0.005f, 1.0f, -1.0f },
                           { "passband", "cutoff", "stopband" });
        }

        { // natural cutoff frequency
            chowdsp::ChebyshevIIFilter<8, FilterType::Lowpass, 60, true, T> filter;
            filter.calcCoefs (Constants::fc, (T) 1 / juce::MathConstants<T>::sqrt2, Constants::fs);

            testFilter<T> (filter,
                           { 100.0f, Constants::fc, 2 * Constants::fc },
                           { 0.0f, -3.0f, -60.0f },
                           { 0.005f, 0.01f, 1.0f },
                           { "passband", "cutoff", "stopband" });
        }
    }

    SECTION ("HPF Test")
    {
        { // Chebyshev cutoff frequency
            chowdsp::ChebyshevIIFilter<8, FilterType::Highpass, 60, false, T> filter;
            filter.calcCoefs (Constants::fc, (T) 1 / juce::MathConstants<T>::sqrt2, Constants::fs);

            testFilter<T> (filter,
                           { 10000.0f, Constants::fc, 0.25f * Constants::fc },
                           { 0.0f, -60.0f, -60.0f },
                           { 0.025f, 4.0f, -1.0f },
                           { "passband", "cutoff", "stopband" });
        }

        { // natural cutoff frequency
            chowdsp::ChebyshevIIFilter<8, FilterType::Highpass, 60, true, T> filter;
            filter.calcCoefs (Constants::fc, (T) 1 / juce::MathConstants<T>::sqrt2, Constants::fs);

            testFilter<T> (filter,
                           { 10000.0f, Constants::fc, 0.5f * Constants::fc },
                           { 0.0f, -3.0f, -60.0f },
                           { 0.025f, 0.01f, 1.0f },
                           { "passband", "cutoff", "stopband" });
        }
    }
}
