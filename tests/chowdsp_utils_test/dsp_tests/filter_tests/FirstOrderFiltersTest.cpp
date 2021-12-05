#include <test_utils.h>
#include <TimedUnitTest.h>

namespace Constants
{
constexpr float fs = 48000.0f;
constexpr float fc = 500.0f;
} // namespace Constants

class FirstOrderFiltersTest : public TimedUnitTest
{
public:
    FirstOrderFiltersTest() : TimedUnitTest ("First Order Filters Test", "Filters")
    {
    }

    template <typename T, typename FilterType>
    void testFrequency (FilterType& filter, T freq, T expGainDB, T maxError, const String& message)
    {
        auto buffer = test_utils::makeSineWave<T> (freq, Constants::fs, (T) 1);

        filter.reset();
        filter.processBlock (buffer.getWritePointer (0), buffer.getNumSamples());

        const auto halfSamples = buffer.getNumSamples() / 2;
        auto magDB = Decibels::gainToDecibels (buffer.getMagnitude (halfSamples, halfSamples));
        expectWithinAbsoluteError (magDB, expGainDB, maxError, message);
    }

    template <typename T>
    void firstOrderLPFTest (T maxError)
    {
        chowdsp::FirstOrderLPF<T> lpFilter;
        lpFilter.calcCoefs ((T) Constants::fc, (T) Constants::fs);

        testFrequency (lpFilter, (T) 1, (T) 0, maxError, "Incorrect gain at low frequencies.");
        testFrequency (lpFilter, (T) Constants::fc, (T) -3.01, maxError, "Incorrect gain at cutoff frequency.");
        testFrequency (lpFilter, Constants::fc * (T) 4, (T) -12.3, (T) 0.1, "Incorrect gain at high frequencies.");
    }

    template <typename T>
    void firstOrderHPFTest (T maxError)
    {
        chowdsp::FirstOrderHPF<T> hpFilter;
        hpFilter.calcCoefs ((T) Constants::fc, (T) Constants::fs);

        testFrequency (hpFilter, (T) Constants::fc / 4, (T) -12.3, (T) 0.1, "Incorrect gain at low frequencies.");
        testFrequency (hpFilter, (T) Constants::fc, (T) -3, maxError, "Incorrect gain at cutoff frequency.");
        testFrequency (hpFilter, (T) Constants::fs * (T) 0.498, (T) 0, maxError, "Incorrect gain at high frequencies.");
    }

    void runTestTimed() override
    {
        beginTest ("First Order LPF Test");
        firstOrderLPFTest<float> (1.0e-2f);
        firstOrderLPFTest<double> (1.0e-2);

        beginTest ("First Order HPF Test");
        firstOrderHPFTest<float> (1.0e-2f);
        firstOrderHPFTest<double> (1.0e-2);
    }
};

static FirstOrderFiltersTest firstOrderFiltersTest;
