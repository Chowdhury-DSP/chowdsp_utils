#include <test_utils.h>
#include <TimedUnitTest.h>

namespace Constants
{
constexpr float fs = 48000.0f;
constexpr float fc = 500.0f;
constexpr float Qval = 0.7071f;
constexpr float gainDB = 6.0f;
} // namespace Constants

class SecondOrderFiltersTest : public TimedUnitTest
{
public:
    SecondOrderFiltersTest() : TimedUnitTest ("Second Order Filters Test", "Filters")
    {
    }

    template <typename T, typename FilterType>
    void testFrequency (FilterType& filter, T freq, T expGainDB, T maxError, const String& message)
    {
        auto buffer = test_utils::makeSineWave<T> (freq, Constants::fs, (T) 2);

        filter.reset();
        filter.processBlock (buffer.getWritePointer (0), buffer.getNumSamples());

        const auto halfSamples = buffer.getNumSamples() / 2;
        auto magDB = Decibels::gainToDecibels (buffer.getMagnitude (halfSamples, halfSamples));
        expectWithinAbsoluteError (magDB, expGainDB, maxError, message);
    }

    template <typename T>
    void peakingFilterTest (T maxError)
    {
        using namespace Constants;

        chowdsp::PeakingFilter<T> filter;
        filter.calcCoefsDB ((T) fc, (T) Qval, (T) gainDB, (T) fs);

        testFrequency (filter, (T) 1, (T) 0, maxError, "Incorrect gain at low frequencies.");
        testFrequency (filter, (T) fc, (T) gainDB, maxError, "Incorrect gain at center frequency.");
        testFrequency (filter, (T) fs * (T) 0.498, (T) 0, maxError, "Incorrect gain at high frequencies.");
    }

    void runTestTimed() override
    {
        beginTest ("Peaking Filter Test");
        peakingFilterTest<float> (1.0e-2f);
        peakingFilterTest<double> (1.0e-3);
    }
};

static SecondOrderFiltersTest secondOrderFiltersTest;
