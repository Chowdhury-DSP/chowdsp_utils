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

    template <typename T>
    void firstOrderLPFTest (T maxError)
    {
        chowdsp::FirstOrderLPF<T> lpFilter;
        lpFilter.calcCoefs ((T) Constants::fc, (T) Constants::fs);

        auto testFrequency = [=, &lpFilter] (T freq, T expGain, const String& message) {
            auto buffer = test_utils::makeSineWave<T> (freq, Constants::fs, (T) 1);

            lpFilter.reset();
            lpFilter.processBlock (buffer.getWritePointer (0), buffer.getNumSamples());

            const auto halfSamples = buffer.getNumSamples() / 2;
            auto mag = buffer.getMagnitude (halfSamples, halfSamples);
            expectWithinAbsoluteError (mag, expGain, maxError, message);
        };

        testFrequency ((T) 10, (T) 1, "Incorrect gain at low frequencies.");
        testFrequency (Constants::fc, Decibels::decibelsToGain ((T) -3), "Incorrect gain at cutoff frequency.");
        testFrequency (Constants::fc * (T) 4, Decibels::decibelsToGain ((T) -12.5), "Incorrect gain at high frequencies.");
    }

    template <typename T>
    void firstOrderHPFTest (T maxError)
    {
        chowdsp::FirstOrderHPF<T> hpFilter;
        hpFilter.calcCoefs ((T) Constants::fc, (T) Constants::fs);

        auto testFrequency = [=, &hpFilter] (T freq, T expGain, const String& message) {
            auto buffer = test_utils::makeSineWave<T> (freq, Constants::fs, (T) 1);

            hpFilter.reset();
            hpFilter.processBlock (buffer.getWritePointer (0), buffer.getNumSamples());

            const auto halfSamples = buffer.getNumSamples() / 2;
            auto mag = buffer.getMagnitude (halfSamples, halfSamples);
            expectWithinAbsoluteError (mag, expGain, maxError, message);
        };

        testFrequency (Constants::fc / 4, Decibels::decibelsToGain ((T) -12.5), "Incorrect gain at low frequencies.");
        testFrequency (Constants::fc, Decibels::decibelsToGain ((T) -3), "Incorrect gain at cutoff frequency.");
        testFrequency (Constants::fc * (T) 10, (T) 1, "Incorrect gain at high frequencies.");
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
