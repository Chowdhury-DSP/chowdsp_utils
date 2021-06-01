#include "test_utils.h"
#include <JuceHeader.h>

namespace Constants
{
constexpr float fs = 48000.0f;
constexpr float fc = 1000.0f;
} // namespace Constants

/** Unit tests for chowdsp::BypassProcessor */
class NthOrderFilterTest : public UnitTest
{
public:
    NthOrderFilterTest() : UnitTest ("Nth Order Filter Test") {}

    using FilterType = chowdsp::StateVariableFilterType;

    template <typename Filter>
    void testFilter (Filter& filt, std::vector<float> freqs, std::vector<float> mags, std::vector<float> errs, const StringArray& messages)
    {
        auto testFrequency = [=, &filt] (float freq, float expGain, float err, const String& message) {
            auto buffer = test_utils::makeSineWave (freq, Constants::fs, 1.0f);

            filt.reset();
            dsp::AudioBlock<float> block (buffer);
            dsp::ProcessContextReplacing<float> ctx (block);
            filt.process (ctx);

            auto halfBlock = buffer.getNumSamples() / 2;
            auto mag = Decibels::gainToDecibels (buffer.getMagnitude (halfBlock, halfBlock));
            expectWithinAbsoluteError (mag, expGain, err, message);
        };

        for (size_t i = 0; i < freqs.size(); ++i)
            testFrequency (freqs[i], mags[i], errs[i], "Incorrect gain at " + messages[(int) i] + " frequency.");
    }

    void lowpassTest()
    {
        chowdsp::NthOrderFilter<float, 4, FilterType::Lowpass> filter;
        filter.prepare ({ Constants::fs, (uint32) Constants::fs, 1 });
        filter.setCutoffFrequency (Constants::fc);

        testFilter (filter,
                    { 100.0f, Constants::fc, 4 * Constants::fc },
                    { 0.0f, -3.0f, -48.0f },
                    { 0.005f, 0.01f, 0.5f },
                    { "passband", "cutoff", "stopband" });
    }

    void highpassTest()
    {
        chowdsp::NthOrderFilter<float, 4, FilterType::Highpass> filter;
        filter.prepare ({ Constants::fs, (uint32) Constants::fs, 1 });
        filter.setCutoffFrequency (Constants::fc);

        testFilter (filter,
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

    void runTest() override
    {
        beginTest ("Lowpass");
        lowpassTest();

        beginTest ("Highpass");
        highpassTest();

        beginTest ("Bandpass");
        bandpassTest();
    }
};

static NthOrderFilterTest nthOrderFilterTest;
