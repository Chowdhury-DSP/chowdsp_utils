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
        REQUIRE_MESSAGE (mag == SIMDApprox<T> ((T) expGain).margin (err), message);
    };

    for (size_t i = 0; i < freqs.size(); ++i)
        testFrequency (freqs[i], mags[i], errs[i], "Incorrect gain at " + messages[i] + " frequency.");
}

TEMPLATE_TEST_CASE ("Nth Order Filter Test", "[dsp][filters][simd]", float, xsimd::batch<float>)
{
    using T = TestType;
    using NumericType = chowdsp::SampleTypeHelpers::NumericType<T>;
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

    SECTION ("Multi-Mode Test")
    {
        static constexpr int Order = 6;
        static constexpr NumericType maxErr = 1.0e-1f;

        auto minus1To1 = test_utils::RandomFloatGenerator { (NumericType) -1, (NumericType) 1 };
        chowdsp::NthOrderFilter<T, Order, chowdsp::StateVariableFilterType::MultiMode> testFilter;
        testFilter.prepare ({ Constants::fs, (uint32_t) 128, 1 });

        auto testCompare = [&] (auto& compareFilter, NumericType mode, NumericType freq, NumericType Q, NumericType gainComp = 1.0f)
        {
            testFilter.reset();
            testFilter.setCutoffFrequency (freq);
            testFilter.setQValue (Q);
            testFilter.setMode (mode);

            compareFilter.prepare ({ Constants::fs, (uint32_t) 128, 1 });
            compareFilter.setCutoffFrequency (freq);
            compareFilter.setQValue (Q);

            for (int i = 0; i < 1000; ++i)
            {
                const auto x = (T) minus1To1();
                const auto actualY = testFilter.processSample (0, x);
                const auto expY = compareFilter.processSample (0, x) * gainComp;
                REQUIRE (actualY == SIMDApprox<T> (expY).margin (maxErr));
            }
        };

        chowdsp::NthOrderFilter<T, Order, chowdsp::StateVariableFilterType::Lowpass> lpf;
        chowdsp::NthOrderFilter<T, Order, chowdsp::StateVariableFilterType::Bandpass> bpf;
        chowdsp::NthOrderFilter<T, Order, chowdsp::StateVariableFilterType::Highpass> hpf;
        const auto bpfGainComp = std::pow (juce::MathConstants<NumericType>::sqrt2, NumericType (Order) / 2);

        testCompare (lpf, (NumericType) 0.0, (NumericType) 1000.0, (NumericType) 2.0);
        testCompare (bpf, (NumericType) 0.5, (NumericType) 500.0, (NumericType) 0.5, bpfGainComp);
        testCompare (hpf, (NumericType) 1.0, (NumericType) 750.0, (NumericType) 1.0);
    }
}
