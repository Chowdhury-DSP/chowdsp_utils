#include <test_utils.h>
#include <TimedUnitTest.h>
#include <chowdsp_filters/chowdsp_filters.h>

namespace Constants
{
constexpr float fs = 48000.0f;
constexpr float fc = 500.0f;
constexpr float Qval = 0.7071f;
} // namespace Constants

class FilterChainTest : public TimedUnitTest
{
public:
    FilterChainTest() : TimedUnitTest ("Filter Chain Test", "Filters")
    {
    }

    void checkBufferLevel (const juce::AudioBuffer<float>& buffer, float expGainDB, float maxError, const juce::String& message)
    {
        const auto halfSamples = buffer.getNumSamples() / 2;
        auto magDB = juce::Decibels::gainToDecibels (buffer.getMagnitude (halfSamples, halfSamples));
        expectWithinAbsoluteError (magDB, expGainDB, maxError, message);
    }

    void secondOrderFilterTest()
    {
        using namespace Constants;

        using FilterType = chowdsp::SecondOrderLPF<float>;
        chowdsp::FilterChain<FilterType, FilterType> filters;
        filters.reset();
        filters.get<0>().calcCoefs (fc, Qval, fs);
        filters.get<1>().calcCoefs (fc, Qval, fs);

        {
            auto buffer = test_utils::makeSineWave<float> (fc, fs, 1.0f);
            auto* bufferPtr = buffer.getWritePointer (0);
            for (int n = 0; n < buffer.getNumSamples(); ++n)
                bufferPtr[n] = filters.processSample (bufferPtr[n]);

            checkBufferLevel (buffer, -6.0f, 0.05f, "SecondOrderLPF processSample() is incorrect!");
        }

        {
            auto buffer = test_utils::makeSineWave<float> (fc, fs, 1.0f);
            filters.processBlock (buffer.getWritePointer (0), buffer.getNumSamples());
            checkBufferLevel (buffer, -6.0f, 0.05f, "SecondOrderLPF processBlock() is incorrect!");
        }
    }

    void nthOrderFilterTest()
    {
        using namespace Constants;

        using FilterType = chowdsp::NthOrderFilter<float, 4>;
        chowdsp::FilterChain<FilterType, FilterType> filters;

        filters.doForEachFilter ([] (auto& filt) {
                                         filt.prepare ({ fs, 100000, 1 });
                                         filt.setCutoffFrequency (fc); });

        auto buffer = test_utils::makeSineWave<float> (2 * fc, fs, 1.0f);
        auto&& block = juce::dsp::AudioBlock<float> { buffer };
        filters.process (chowdsp::ProcessContextReplacing<float> { block });

        checkBufferLevel (buffer, -48.0f, 0.5f, "NthOrderFilter process() is incorrect!");
    }

    void runTestTimed() override
    {
        beginTest ("Second Order Filter Test");
        secondOrderFilterTest();

        beginTest ("Nth Order Filter Test");
        nthOrderFilterTest();
    }
};

static FilterChainTest filterChainTest;
