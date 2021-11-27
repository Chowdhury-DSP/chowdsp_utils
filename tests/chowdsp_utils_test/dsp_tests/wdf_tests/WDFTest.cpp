#include <test_utils.h>
#include <TimedUnitTest.h>
#include "CommonWDFTests.h"

namespace
{
constexpr double _fs = 44100.0;
constexpr double fc = 500.0;
constexpr float error = 0.1f;
} // namespace

using namespace chowdsp::WDF;

/** Unit tests for chowdsp WDF classes. Includes test for:
 *   - Voltage divider circuit
 *   - Current divider circuit
 *   - RC Lowpass filter circuit
 *   - RLC Highpass filter circuit with alpha transform
 *   - Element impedance change and propagation of impedance changes
 * 
 * @TODO: add tests for current divider
 */
class WDFTest : public TimedUnitTest
{
public:
    WDFTest() : TimedUnitTest ("Wave Digital Filter Test", "Wave Digital Filters") {}

    template <typename FloatType>
    void currentSwitchTest()
    {
        Resistor<FloatType> r1 ((FloatType) 10000.0);
        ResistiveCurrentSource<FloatType> Is;

        WDFSeries<FloatType> s1 (&r1, &Is);
        Switch<FloatType> sw { &s1 };

        // run with switch closed
        sw.setClosed (true);
        Is.setCurrent ((FloatType) 1.0);
        sw.incident (s1.reflected());
        s1.incident (sw.reflected());

        auto currentClosed = r1.current();
        expectWithinAbsoluteError (currentClosed, (FloatType) -1.0, (FloatType) 1.0e-3, "Current with switch closed is incorrect!");

        // run with switch open
        sw.setClosed (false);
        sw.incident (s1.reflected());
        s1.incident (sw.reflected());

        auto currentOpen = r1.current();
        expectEquals (currentOpen, (FloatType) 0.0, "Current with switch open is incorrect!");
    }

    template <typename FloatType>
    void yParameterTest()
    {
        constexpr auto y11 = (FloatType) 0.11;
        constexpr auto y12 = (FloatType) 0.22;
        constexpr auto y21 = (FloatType) 0.33;
        constexpr auto y22 = (FloatType) 0.44;
        constexpr auto voltage = (FloatType) 2.0;

        Resistor<FloatType> res { (FloatType) 10000.0 };
        YParameter<FloatType> yParam { &res, y11, y12, y21, y22 };
        IdealVoltageSource<FloatType> Vs { &yParam };

        Vs.setVoltage (voltage);
        Vs.incident (yParam.reflected());
        yParam.incident (Vs.reflected());

        expectWithinAbsoluteError (-res.current(), y11 * res.voltage() + y12 * voltage, (FloatType) 1.0e-3, "Y-Parameter current 1 is incorrect");
        expectWithinAbsoluteError (yParam.current(), y21 * res.voltage() + y22 * voltage, (FloatType) 1.0e-3, "Y-Parameter current 2 is incorrect");
    }

    void rcLowpassTest()
    {
        using FloatType = double;
        constexpr auto capValue = (FloatType) 1.0e-6;
        constexpr auto resValue = (FloatType) 1.0 / (MathConstants<FloatType>::twoPi * (FloatType) fc * capValue);

        Capacitor<FloatType> c1 (capValue, (FloatType) _fs);
        Resistor<FloatType> r1 (resValue);

        WDFSeries<FloatType> s1 (&r1, &c1);
        PolarityInverter<FloatType> p1 (&s1);
        IdealVoltageSource<FloatType> vs { &p1 };

        auto processBuffer = [&] (float* buffer, const int numSamples) {
            for (int n = 0; n < numSamples; ++n)
            {
                vs.setVoltage ((FloatType) buffer[n]);

                vs.incident (p1.reflected());
                p1.incident (vs.reflected());

                buffer[n] = (float) c1.voltage();
            }
        };

        auto getMagDB = [=] (const AudioBuffer<float>& buffer) -> float {
            return Decibels::gainToDecibels (buffer
                                                 .getMagnitude (1000, buffer.getNumSamples() - 1000));
        };

        auto sineDouble = test_utils::makeSineWave ((float) fc * 2.0f, (float) _fs, 1.0f);
        processBuffer (sineDouble.getWritePointer (0), sineDouble.getNumSamples());
        auto magDouble = getMagDB (sineDouble);
        expectWithinAbsoluteError (magDouble, -7.0f, error, "RC Lowpass incorrect voltage at 2x fc");

        auto sineFc = test_utils::makeSineWave ((float) fc, (float) _fs, 1.0f);
        processBuffer (sineFc.getWritePointer (0), sineFc.getNumSamples());
        auto magFc = getMagDB (sineFc);
        expectWithinAbsoluteError (magFc, -3.0f, error, "RC Lowpass incorrect voltage at fc");

        auto sineHalf = test_utils::makeSineWave ((float) fc * 0.5f, (float) _fs, 1.0f);
        processBuffer (sineHalf.getWritePointer (0), sineHalf.getNumSamples());
        auto magHalf = getMagDB (sineHalf);
        expectWithinAbsoluteError (magHalf, -1.0f, error, "RC Lowpass incorrect voltage at 0.5x fc");
    }

    void alphaTransformTest()
    {
        // 1 kHz cutoff 2nd-order highpass
        constexpr float R = 300.0f;
        constexpr float C = 1.0e-6f;
        constexpr float L = 0.022f;

        auto processBuffer = [] (float* buffer, const int numSamples, auto& vs, auto& p1, auto& l1) {
            for (int n = 0; n < numSamples; ++n)
            {
                vs.setVoltage (buffer[n]);

                vs.incident (p1.reflected());
                p1.incident (vs.reflected());

                buffer[n] = l1.voltage();
            }
        };

        auto getMagDB = [=] (const AudioBuffer<float>& buffer) -> float {
            return Decibels::gainToDecibels (buffer
                                                 .getMagnitude (1000, buffer.getNumSamples() - 1000));
        };

        // reference filter
        float refMag;
        {
            Capacitor<float> c1 (C);
            Resistor<float> r1 (R);
            Inductor<float> l1 (L);

            WDFSeries<float> s1 (&r1, &c1);
            WDFSeries<float> s2 (&s1, &l1);
            PolarityInverter<float> p1 (&s2);
            IdealVoltageSource<float> vs { &p1 };

            c1.prepare ((float) _fs);
            l1.prepare ((float) _fs);

            auto refSine = test_utils::makeSineWave (10.0e3f, (float) _fs, 1.0f);
            processBuffer (refSine.getWritePointer (0), refSine.getNumSamples(), vs, p1, l1);
            refMag = getMagDB (refSine);
            expectWithinAbsoluteError (refMag, 0.0f, 0.1f, "Reference highpass passband gain is incorrect!");
        }

        CapacitorAlpha<float> c1 (C);
        Resistor<float> r1 (R);
        InductorAlpha<float> l1 (L);

        WDFSeries<float> s1 (&r1, &c1);
        WDFSeries<float> s2 (&s1, &l1);
        PolarityInverter<float> p1 (&s2);
        IdealVoltageSource<float> vs { &p1 };

        // alpha = 1.0 filter
        {
            constexpr float alpha = 1.0f;
            c1.prepare ((float) _fs);
            c1.setAlpha (alpha);
            l1.prepare ((float) _fs);
            l1.setAlpha (alpha);

            auto a1Sine = test_utils::makeSineWave (10.0e3f, (float) _fs, 1.0f);
            processBuffer (a1Sine.getWritePointer (0), a1Sine.getNumSamples(), vs, p1, l1);
            auto a1Mag = getMagDB (a1Sine);
            expectWithinAbsoluteError (a1Mag, 0.0f, 0.1f, "Alpha = 1 highpass passband gain is incorrect!");
            expectWithinAbsoluteError (a1Mag, refMag, 1.0e-6f, "Alpha = 1 passband gain is not equal to reference!");
        }

        // alpha = 0.1 filter
        {
            constexpr float alpha = 0.1f;
            c1.reset();
            c1.setAlpha (alpha);
            l1.reset();
            l1.setAlpha (alpha);

            auto a01Sine = test_utils::makeSineWave (10.0e3f, (float) _fs, 1.0f);
            processBuffer (a01Sine.getWritePointer (0), a01Sine.getNumSamples(), vs, p1, l1);
            auto a01Mag = getMagDB (a01Sine);
            expectWithinAbsoluteError (a01Mag, refMag - 1.1f, 0.1f, "Alpha = 0.1 passband gain is incorrect!");
        }
    }

    void impedanceChangeTest()
    {
        auto checkImpedanceChange = [=] (auto component, const String& name, float value1, float value2, auto changeFunc, auto impedanceCalc) {
            expectEquals (component.R, impedanceCalc (value1), "Initial " + name + " impedance incorrect!");

            changeFunc (component, value2);
            expectEquals (component.R, impedanceCalc (value2), "Changed " + name + " impedance incorrect!");
        };

        auto checkImpedanceProp = [=] (auto component, const String& name, float value1, float value2, auto changeFunc, auto impedanceCalc) {
            constexpr float otherR = 5000.0f;
            Resistor<float> r2 { otherR };
            WDFSeries<float> s1 (&component, &r2);
            IdealCurrentSource<float> is (&s1);
            is.setCurrent (1.0f);

            expectEquals (s1.R, impedanceCalc (value1) + otherR, "Initial " + name + " propagated impedance incorrect!");
            expectEquals (is.reflected(), 2.0f * s1.R, "Initial " + name + " propagated root impedance incorrect!");

            changeFunc (component, value2);
            expectEquals (s1.R, impedanceCalc (value2) + otherR, "Changed " + name + " propagated impedance incorrect!");
            expectEquals (is.reflected(), 2.0f * s1.R, "Changed " + name + " propagated root impedance incorrect!");
        };

        auto doImpedanceChecks = [=] (auto... params) {
            checkImpedanceChange (params...);
            checkImpedanceProp (params...);
        };

        // resistor
        doImpedanceChecks (
            Resistor<float> { 1000.0f }, "Resistor", 1000.0f, 2000.0f, [=] (Resistor<float>& r, float value) { r.setResistanceValue (value); }, [=] (float value) { return value; });

        // capacitor
        doImpedanceChecks (
            Capacitor<float> { 1.0e-6f, _fs }, "Capacitor", 1.0e-6f, 2.0e-6f, [=] (Capacitor<float>& c, float value) { c.setCapacitanceValue (value); }, [=] (float value) { return 1.0f / (2.0f * value * (float) _fs); });

        // capacitor alpha
        doImpedanceChecks (
            CapacitorAlpha<float> { 1.0e-6f, _fs, 0.5f }, "CapacitorAlpha", 1.0e-6f, 2.0e-6f, [=] (CapacitorAlpha<float>& c, float value) { c.setCapacitanceValue (value); }, [=] (float value) { return 1.0f / (1.5f * value * (float) _fs); });

        // inductor
        doImpedanceChecks (
            Inductor<float> { 1.0f, _fs }, "Inductor", 1.0f, 2.0f, [=] (Inductor<float>& i, float value) { i.setInductanceValue (value); }, [=] (float value) { return 2.0f * value * (float) _fs; });

        // inductor alpha
        doImpedanceChecks (
            InductorAlpha<float> { 1.0f, _fs, 0.5f }, "InductorAlpha", 1.0f, 2.0f, [=] (InductorAlpha<float>& i, float value) { i.setInductanceValue (value); }, [=] (float value) { return 1.5f * value * (float) _fs; });

        // resistive voltage source
        doImpedanceChecks (
            ResistiveVoltageSource<float> { 1000.0f }, "ResistiveVoltageSource", 1000.0f, 2000.0f, [=] (ResistiveVoltageSource<float>& r, float value) { r.setResistanceValue (value); }, [=] (float value) { return value; });

        // resistive current source
        doImpedanceChecks (
            ResistiveCurrentSource<float> { 1000.0f }, "ResistiveCurrentSource", 1000.0f, 2000.0f, [=] (ResistiveCurrentSource<float>& r, float value) { r.setResistanceValue (value); }, [=] (float value) { return value; });
    }

    void runTestTimed() override
    {
        beginTest ("Voltage Divider Test");
        voltageDividerTest<float> (*this);
        voltageDividerTest<double> (*this);

        beginTest ("Current Divider Test");
        currentDividerTest<float> (*this);
        currentDividerTest<double> (*this);

        beginTest ("Current Switch Test");
        currentSwitchTest<float>();
        currentSwitchTest<double>();

        beginTest ("Y-Parameter Test");
        yParameterTest<float>();
        yParameterTest<double>();

        beginTest ("RC Lowpass Test");
        rcLowpassTest();

        beginTest ("Alpha Transform Test");
        alphaTransformTest();

        beginTest ("Impedance Change Test");
        impedanceChangeTest();
    }
};

static WDFTest wdfTest;
