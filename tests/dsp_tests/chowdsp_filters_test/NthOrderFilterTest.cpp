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
    auto testFrequency = [&filt] (float freq, float expGain, float err, const std::string& message) {
        auto buffer = test_utils::makeSineWave<T> (freq, Constants::fs, 1.0f);

        filt.reset();
        filt.processBlock (buffer);

        const auto halfSamples = buffer.getNumSamples() / 2;
        auto mag = chowdsp::SIMDUtils::gainToDecibels (chowdsp::BufferMath::getMagnitude (buffer, halfSamples, halfSamples));
        REQUIRE_MESSAGE (mag == SIMDApprox<T> ((T) expGain).margin (err), message);
    };

    for (size_t i = 0; i < freqs.size(); ++i)
        testFrequency (freqs[i], mags[i], errs[i], "Incorrect gain at " + messages[i] + " frequency.");
}

TEMPLATE_TEST_CASE ("Nth Order Filter Test", "", float, xsimd::batch<float>)
{
    using T = TestType;
    using FilterType = chowdsp::StateVariableFilterType;

    SECTION ("LPF Test")
    {
        chowdsp::NthOrderFilter<T, 4, FilterType::Lowpass> filter;
        filter.prepare ({ Constants::fs, (juce::uint32) Constants::fs, 1 });
        filter.setCutoffFrequency (Constants::fc);

        testFilter<T> (filter,
                       { 100.0f, Constants::fc, 4 * Constants::fc },
                       { 0.0f, -3.0f, -48.0f },
                       { 0.005f, 0.01f, 0.5f },
                       { "passband", "cutoff", "stopband" });
    }

    SECTION ("HPF Test")
    {
        chowdsp::NthOrderFilter<T, 4, FilterType::Highpass> filter;
        filter.prepare ({ Constants::fs, (juce::uint32) Constants::fs, 1 });
        filter.setCutoffFrequency (Constants::fc);

        testFilter<T> (filter,
                       { 10000.0f, Constants::fc, 0.25f * Constants::fc },
                       { 0.0f, -3.0f, -48.0f },
                       { 0.01f, 0.01f, 0.5f },
                       { "passband", "cutoff", "stopband" });
    }

    SECTION ("BPF Test")
    {
        INFO ("TODO...");
        // using namespace Constants;
        // chowdsp::NthOrderFilter<float, 4, FilterType::Bandpass> filter;
        // filter.prepare ({ fs, (uint32) fs, 1 });
        // filter.setCutoffFrequency (fc);
        // filter.setQValue (fc / bandWidth);

        // testFilter (filter,
        //             { fc, fc + bandWidth / 2, fc - bandWidth / 2 },
        //             { 0.0f, -3.0f, -3.0f },
        //             { 0.0001f, 0.01f, 0.01f },
        //             { "passband", "high cutoff", "low cutoff" });
    }
}
