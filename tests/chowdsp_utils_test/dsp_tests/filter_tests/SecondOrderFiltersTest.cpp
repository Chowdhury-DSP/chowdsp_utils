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
    void secondOrderLPFTest (T maxError)
    {
        using namespace Constants;

        chowdsp::SecondOrderLPF<T> filter;
        filter.calcCoefs ((T) fc, (T) Qval, (T) fs);

        testFrequency (filter, (T) 1, (T) 0, maxError, "Incorrect gain at low frequencies.");
        testFrequency (filter, (T) fc, (T) -3.01, maxError, "Incorrect gain at cutoff frequency.");
        testFrequency (filter, (T) fc * (T) 4, (T) -24.2, (T) 0.1, "Incorrect gain at high frequencies.");
    }

    template <typename T>
    void secondOrderHPFTest (T maxError)
    {
        using namespace Constants;

        chowdsp::SecondOrderHPF<T> filter;
        filter.calcCoefs ((T) fc, (T) Qval, (T) fs);

        testFrequency (filter, (T) fc / (T) 4, (T) -24.0, (T) 0.1, "Incorrect gain at low frequencies.");
        testFrequency (filter, (T) fc, (T) -3.0, maxError, "Incorrect gain at cutoff frequency.");
        testFrequency (filter, (T) fs * (T) 0.498, (T) 0, maxError, "Incorrect gain at high frequencies.");
    }

    template <typename T>
    void secondOrderBPFTest (T maxError)
    {
        using namespace Constants;

        chowdsp::SecondOrderBPF<T> filter;
        filter.calcCoefs ((T) fc, (T) Qval, (T) fs);

        testFrequency (filter, (T) fc / (T) 4, (T) -9.0, (T) 0.1, "Incorrect gain at low frequencies.");
        testFrequency (filter, (T) fc, (T) 0, maxError, "Incorrect gain at center frequency.");
        testFrequency (filter, (T) fc * (T) 4, (T) -9.1, (T) 0.1, "Incorrect gain at high frequencies.");
    }

    template <typename T>
    void notchFilterTest (T maxError)
    {
        using namespace Constants;

        chowdsp::NotchFilter<T> filter;
        filter.calcCoefs ((T) fc, (T) Qval, (T) fs);

        testFrequency (filter, (T) 1, (T) 0, maxError, "Incorrect gain at low frequencies.");
        testFrequency (filter, (T) fc, (T) -60, (T) 5, "Incorrect gain at center frequency.");
        testFrequency (filter, (T) fs * (T) 0.498, (T) 0, maxError, "Incorrect gain at high frequencies.");
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

    template <typename T>
    void lowShelfFilterTest (T maxError)
    {
        using namespace Constants;

        chowdsp::LowShelfFilter<T> filter;
        filter.calcCoefsDB ((T) fc, (T) Qval, (T) gainDB, (T) fs);

        testFrequency (filter, (T) 1, (T) gainDB, maxError, "Incorrect gain at low frequencies.");
        testFrequency (filter, (T) fc, (T) gainDB * (T) 0.5, maxError, "Incorrect gain at cutoff frequency.");
        testFrequency (filter, (T) fs * (T) 0.498, (T) 0, maxError, "Incorrect gain at high frequencies.");
    }

    template <typename T>
    void highShelfFilterTest (T maxError)
    {
        using namespace Constants;

        chowdsp::HighShelfFilter<T> filter;
        filter.calcCoefsDB ((T) fc, (T) Qval, (T) gainDB, (T) fs);

        testFrequency (filter, (T) 1, (T) 0, maxError, "Incorrect gain at low frequencies.");
        testFrequency (filter, (T) fc, (T) gainDB * (T) 0.5, maxError, "Incorrect gain at cutoff frequency.");
        testFrequency (filter, (T) fs * (T) 0.498, (T) gainDB, maxError, "Incorrect gain at high frequencies.");
    }

    void runTestTimed() override
    {
        beginTest ("Second Order LPF Test");
        secondOrderLPFTest<float> (1.0e-2f);
        secondOrderLPFTest<double> (1.0e-2);

        beginTest ("Second Order HPF Test");
        secondOrderHPFTest<float> (1.0e-2f);
        secondOrderHPFTest<double> (1.0e-2);

        beginTest ("Second Order BPF Test");
        secondOrderBPFTest<float> (1.0e-2f);
        secondOrderBPFTest<double> (1.0e-2);

        beginTest ("Notch Filter Test");
        notchFilterTest<float> (1.0e-2f);
        notchFilterTest<double> (1.0e-3);

        beginTest ("Peaking Filter Test");
        peakingFilterTest<float> (1.0e-2f);
        peakingFilterTest<double> (1.0e-3);

        beginTest ("Low Shelf Filter Test");
        lowShelfFilterTest<float> (1.0e-2f);
        lowShelfFilterTest<double> (1.0e-2);

        beginTest ("High Shelf Filter Test");
        highShelfFilterTest<float> (1.0e-2f);
        highShelfFilterTest<double> (1.0e-2);
    }
};

static SecondOrderFiltersTest secondOrderFiltersTest;
