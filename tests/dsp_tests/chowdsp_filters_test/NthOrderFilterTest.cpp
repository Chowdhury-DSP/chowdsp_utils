#include <test_utils.h>
#include <TimedUnitTest.h>
#include <chowdsp_filters/chowdsp_filters.h>

namespace Constants
{
constexpr float fs = 48000.0f;
constexpr float fc = 1000.0f;
} // namespace Constants

class NthOrderFilterTest : public TimedUnitTest
{
public:
    NthOrderFilterTest() : TimedUnitTest ("Nth Order Filter Test", "Filters") {}

    using FilterType = chowdsp::StateVariableFilterType;

    template <typename T, typename Filter>
    void testFilter (Filter& filt, std::vector<float> freqs, std::vector<float> mags, std::vector<float> errs, const juce::StringArray& messages)
    {
        auto testFrequency = [=, &filt] (float freq, float expGain, float err, const juce::String& message) {
            auto buffer = test_utils::makeSineWave (freq, Constants::fs, 1.0f);

            juce::HeapBlock<char> dataBlock;
            auto block = test_utils::bufferToBlock<T> (dataBlock, buffer);

            filt.reset();
            chowdsp::ProcessContextReplacing<T> ctx (block);
            filt.process (ctx);

            test_utils::blockToBuffer<T> (buffer, block);
            auto halfBlock = buffer.getNumSamples() / 2;
            auto mag = juce::Decibels::gainToDecibels (buffer.getMagnitude (halfBlock, halfBlock));
            expectWithinAbsoluteError (mag, expGain, err, message);
        };

        for (size_t i = 0; i < freqs.size(); ++i)
            testFrequency (freqs[i], mags[i], errs[i], "Incorrect gain at " + messages[(int) i] + " frequency.");
    }

    template <typename T>
    void lowpassTest()
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

    template <typename T>
    void highpassTest()
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

    void bandpassTest()
    {
        std::cout << "TODO..." << std::endl;
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

    void runTestTimed() override
    {
        beginTest ("Lowpass");
        lowpassTest<float>();

        beginTest ("Lowpass SIMD");
        lowpassTest<xsimd::batch<float>>();

        beginTest ("Highpass");
        highpassTest<float>();

        beginTest ("Highpass SIMD");
        highpassTest<xsimd::batch<float>>();

        beginTest ("Bandpass");
        bandpassTest();
    }
};

static NthOrderFilterTest nthOrderFilterTest;
