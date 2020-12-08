#include <JuceHeader.h>
#include "test_utils.h"

namespace
{
    constexpr double fs = 44100.0;
    constexpr double fc = 500.0;
    constexpr float error = 0.1f;
}

/** Unit tests for chowdsp WDF classes. Includes test for:
 *   - Voltage divider circuit
 *   - RC Lowpass filter circuit
 */
class WDFTest : public UnitTest
{
public:
    WDFTest() : UnitTest ("Wave Digital Filter Test") {}

    void voltageDiviverTest()
    {
        chowdsp::WDF::Resistor r1 (10000.0);
        chowdsp::WDF::Resistor r2 (10000.0);
        chowdsp::WDF::IdealVoltageSource vs;

        chowdsp::WDF::WDFSeries s1 (&r1, &r2);
        chowdsp::WDF::PolarityInverter p1 (&s1);
        vs.connectToNode (&p1);

        vs.setVoltage (10.0f);
        vs.incident (p1.reflected());
        p1.incident (vs.reflected());

        auto vOut = r2.voltage();

        expectEquals (vOut, 5.0, "Voltage divider: incorrect voltage!");
    }

    void rcLowpassTest()
    {
        constexpr double capValue = 1.0e-6;
        constexpr double resValue = 1.0 / (MathConstants<double>::twoPi * fc * capValue);

        chowdsp::WDF::Capacitor c1 (capValue, fs);
        chowdsp::WDF::Resistor r1 (resValue);

        chowdsp::WDF::WDFSeries s1 (&r1, &c1);
        chowdsp::WDF::PolarityInverter p1 (&s1);

        chowdsp::WDF::IdealVoltageSource vs;
        vs.connectToNode (&p1);

        auto processBuffer = [&] (float* buffer, const int numSamples)
        {
            for (int n = 0; n < numSamples; ++n)
            {
                vs.setVoltage ((double) buffer[n]);

                vs.incident (p1.reflected());
                p1.incident (vs.reflected());

                buffer[n] = (float) c1.voltage();
            }
        };

        auto getMagDB = [=] (const AudioBuffer<float>& buffer) -> float
        {
            return Decibels::gainToDecibels (buffer
                .getMagnitude (1000, buffer.getNumSamples() - 1000));
        };

        auto sineDouble = test_utils::makeSineWave ((float) fc * 2.0f, (float) fs, 1.0f);
        processBuffer (sineDouble.getWritePointer (0), sineDouble.getNumSamples());
        auto magDouble = getMagDB (sineDouble);
        expectWithinAbsoluteError (magDouble, -7.0f, error, "RC Lowpass incorrect voltage at 2x fc");

        auto sineFc = test_utils::makeSineWave ((float) fc, (float) fs, 1.0f);
        processBuffer (sineFc.getWritePointer (0), sineFc.getNumSamples());
        auto magFc = getMagDB (sineFc);
        expectWithinAbsoluteError (magFc, -3.0f, error, "RC Lowpass incorrect voltage at fc");

        auto sineHalf = test_utils::makeSineWave ((float) fc * 0.5f, (float) fs, 1.0f);
        processBuffer (sineHalf.getWritePointer (0), sineHalf.getNumSamples());
        auto magHalf = getMagDB (sineHalf);
        expectWithinAbsoluteError (magHalf, -1.0f, error, "RC Lowpass incorrect voltage at 0.5x fc");
    }

    void runTest() override
    {
        beginTest ("Voltage Divider Test");
        voltageDiviverTest();

        beginTest ("RC Lowpass Test");
        rcLowpassTest();        
    }
};

static WDFTest mfTest;
