#include <TimedUnitTest.h>
#include <test_utils.h>
#include <chowdsp_filters/chowdsp_filters.h>

namespace Constants
{
constexpr float fs = 48000.0f;
constexpr float fc = 1000.0f;
} // namespace Constants

class FractionalOrderFilterTest : public TimedUnitTest
{
public:
    FractionalOrderFilterTest() : TimedUnitTest ("Fractional Order Filter Test", "Filters") {}

    template <typename T, typename FilterType, typename NumericType = chowdsp::SampleTypeHelpers::NumericType<T>>
    void testFrequency (FilterType& filter, NumericType freq, NumericType expGainDB, NumericType maxError, const juce::String& message)
    {
        auto buffer = test_utils::makeSineWave<NumericType> (freq, Constants::fs, (NumericType) 1);

        juce::HeapBlock<char> dataBlock;
        auto block = test_utils::bufferToBlock<T> (dataBlock, buffer);

        filter.reset();
        filter.processBlock (block.getChannelPointer (0), buffer.getNumSamples());

        test_utils::blockToBuffer<T> (buffer, block);
        const auto halfSamples = buffer.getNumSamples() / 2;
        auto magDB = juce::Decibels::gainToDecibels (buffer.getMagnitude (halfSamples, halfSamples));
        expectWithinAbsoluteError (magDB, expGainDB, maxError, message);
    }

    template <typename T, typename NumericType = chowdsp::SampleTypeHelpers::NumericType<T>>
    void firstOrderLPFTest (NumericType maxError)
    {
        chowdsp::FractionalOrderFilter<T> lpFilter;
        lpFilter.prepare ((double) Constants::fs, 1);
        lpFilter.calcCoefs ((T) Constants::fc, (T) 0.5, (NumericType) Constants::fs);

        testFrequency<T> (lpFilter, (NumericType) 1, (NumericType) 0, maxError, "Incorrect gain at low frequencies.");
        testFrequency<T> (lpFilter, (NumericType) Constants::fc, (NumericType) -1.46, maxError, "Incorrect gain at cutoff frequency.");
        testFrequency<T> (lpFilter, Constants::fc * (NumericType) 4, (NumericType) -6.1, (NumericType) 0.1, "Incorrect gain at high frequencies.");
    }

    void runTestTimed() override
    {
        beginTest ("Fractional-Order LPF Test");
        firstOrderLPFTest<float> (1.0e-2f);
        firstOrderLPFTest<double> (1.0e-2);
    }
};

static FractionalOrderFilterTest fractionalOrderFilterTest;
