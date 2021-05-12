#include "../test_utils.h"
#include <JuceHeader.h>

namespace
{
constexpr double fs = 44100.0;
constexpr double fc = 500.0;
constexpr float error = 0.1f;
} // namespace

using namespace chowdsp::WDF;

/** Unit tests for chowdsp WDF classes. Includes test for:
 *   - Voltage divider circuit
 *   - RC Lowpass filter circuit
 * 
 *  @TODO: SIMD test, test w/ templated adaptors
 */
class WDFTest : public UnitTest
{
public:
    WDFTest() : UnitTest ("Wave Digital Filter Test") {}

    void voltageDiviverTest()
    {
        using FloatType = float;

        Resistor<FloatType> r1 ((FloatType) 10000.0);
        Resistor<FloatType> r2 ((FloatType) 10000.0);
        IdealVoltageSource<FloatType> vs;

        WDFSeries<FloatType> s1 (&r1, &r2);
        PolarityInverter<FloatType> p1 (&s1);
        vs.connectToNode (&p1);

        vs.setVoltage ((FloatType) 10.0f);
        vs.incident (p1.reflected());
        p1.incident (vs.reflected());

        auto vOut = r2.voltage();

        expectEquals (vOut, (FloatType) 5.0, "Voltage divider: incorrect voltage!");
    }

    void rcLowpassTest()
    {
        using FloatType = double;
        constexpr auto capValue = (FloatType) 1.0e-6;
        constexpr auto resValue = (FloatType) 1.0 / (MathConstants<FloatType>::twoPi * (FloatType) fc * capValue);

        Capacitor<FloatType> c1 (capValue, (FloatType) fs);
        Resistor<FloatType> r1 (resValue);

        WDFSeries<FloatType> s1 (&r1, &c1);
        PolarityInverter<FloatType> p1 (&s1);

        IdealVoltageSource<FloatType> vs;
        vs.connectToNode (&p1);

        auto processBuffer = [&] (float* buffer, const int numSamples) {
            for (int n = 0; n < numSamples; ++n)
            {
                vs.setVoltage ((double) buffer[n]);

                vs.incident (p1.reflected());
                p1.incident (vs.reflected());

                buffer[n] = (float) c1.voltage();
            }
        };

        auto getMagDB = [=] (const AudioBuffer<float>& buffer) -> float {
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
