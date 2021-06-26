#include "test_utils.h"

namespace
{
constexpr double _fs = 44100.0;
constexpr double fc = 500.0;
constexpr float error = 0.1f;
} // namespace

using namespace chowdsp;
using namespace chowdsp::WDFT;

class StaticWDFTest : public UnitTest
{
public:
    StaticWDFTest() : UnitTest ("Static Wave Digital Filter Test") {}

    template <typename FloatType>
    void voltageDividerTest()
    {
        using namespace chowdsp::WDFT;
        ResistorT<FloatType> r1 ((FloatType) 10000.0);
        ResistorT<FloatType> r2 ((FloatType) 10000.0);

        auto s1 = makeSeries<FloatType> (r1, r2);
        auto p1 = makeInverter<FloatType> (s1);
        IdealVoltageSourceT<FloatType, decltype (p1)> vs { p1 };

        vs.setVoltage ((FloatType) 10.0f);
        vs.incident (p1.reflected());
        p1.incident (vs.reflected());

        auto vOut = voltage<FloatType> (r2);

        if (vOut != (FloatType) 5.0)
            expect (false, "Voltage divider: incorrect voltage!");
    }

    template <typename FloatType>
    void currentDividerTest()
    {
        using namespace chowdsp::WDF;
        ResistorT<FloatType> r1 ((FloatType) 10000.0);
        ResistorT<FloatType> r2 ((FloatType) 10000.0);

        auto p1 = makeParallel<FloatType> (r1, r2);
        IdealCurrentSourceT<FloatType, decltype (p1)> is { p1 };

        is.setCurrent ((FloatType) 1.0f);
        is.incident (p1.reflected());
        p1.incident (is.reflected());

        auto iOut = current<FloatType> (r2);
        expectEquals (iOut, (FloatType) 0.5, "Current divider: incorrect current!");
    }

    void rcLowpassTest()
    {
        using FloatType = double;
        constexpr auto capValue = (FloatType) 1.0e-6;
        constexpr auto resValue = (FloatType) 1.0 / (MathConstants<FloatType>::twoPi * (FloatType) fc * capValue);

        CapacitorT<FloatType> c1 (capValue, (FloatType) _fs);
        ResistorT<FloatType> r1 (resValue);

        WDFSeriesT<FloatType, ResistorT<FloatType>, CapacitorT<FloatType>> s1 { r1, c1 };
        PolarityInverterT<FloatType, WDFSeriesT<FloatType, ResistorT<FloatType>, CapacitorT<FloatType>>> p1 { s1 };

        IdealVoltageSourceT<FloatType, decltype (p1)> vs { p1 };

        auto processBuffer = [&] (float* buffer, const int numSamples) {
            for (int n = 0; n < numSamples; ++n)
            {
                vs.setVoltage ((FloatType) buffer[n]);

                vs.incident (p1.reflected());
                p1.incident (vs.reflected());

                buffer[n] = (float) voltage<FloatType> (c1);
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

                buffer[n] = voltage<float> (l1);
            }
        };

        auto getMagDB = [=] (const AudioBuffer<float>& buffer) -> float {
            return Decibels::gainToDecibels (buffer
                                                 .getMagnitude (1000, buffer.getNumSamples() - 1000));
        };

        // reference filter
        float refMag;
        {
            CapacitorT<float> c1 (C, (float) _fs);
            ResistorT<float> r1 (R);
            InductorT<float> l1 (L, (float) _fs);

            WDFSeriesT<float, ResistorT<float>, CapacitorT<float>> s1 { r1, c1 };
            WDFSeriesT<float, WDFSeriesT<float, ResistorT<float>, CapacitorT<float>>, InductorT<float>> s2 { s1, l1 };
            PolarityInverterT<float, WDFSeriesT<float, WDFSeriesT<float, ResistorT<float>, CapacitorT<float>>, InductorT<float>>> p1 { s2 };

            IdealVoltageSourceT<float, decltype (p1)> vs { p1 };

            auto refSine = test_utils::makeSineWave (10.0e3f, (float) _fs, 1.0f);
            processBuffer (refSine.getWritePointer (0), refSine.getNumSamples(), vs, p1, l1);
            refMag = getMagDB (refSine);
            expectWithinAbsoluteError (refMag, 0.0f, 0.1f, "Reference highpass passband gain is incorrect!");
        }

        // alpha = 1.0 filter
        {
            constexpr float alpha = 1.0f;
            CapacitorAlphaT<float> c1 (C, (float) _fs, alpha);
            ResistorT<float> r1 (R);
            InductorAlphaT<float> l1 (L, (float) _fs, alpha);

            WDFSeriesT<float, ResistorT<float>, CapacitorAlphaT<float>> s1 { r1, c1 };
            WDFSeriesT<float, WDFSeriesT<float, ResistorT<float>, CapacitorAlphaT<float>>, InductorAlphaT<float>> s2 { s1, l1 };
            PolarityInverterT<float, WDFSeriesT<float, WDFSeriesT<float, ResistorT<float>, CapacitorAlphaT<float>>, InductorAlphaT<float>>> p1 { s2 };

            IdealVoltageSourceT<float, decltype (p1)> vs { p1 };

            auto a1Sine = test_utils::makeSineWave (10.0e3f, (float) _fs, 1.0f);
            processBuffer (a1Sine.getWritePointer (0), a1Sine.getNumSamples(), vs, p1, l1);
            auto a1Mag = getMagDB (a1Sine);
            expectWithinAbsoluteError (a1Mag, 0.0f, 0.1f, "Alpha = 1 highpass passband gain is incorrect!");
            expectWithinAbsoluteError (a1Mag, refMag, 1.0e-6f, "Alpha = 1 passband gain is not equal to reference!");
        }

        // alpha = 0.1 filter
        {
            constexpr float alpha = 0.1f;
            CapacitorAlphaT<float> c1 (C, (float) _fs, alpha);
            ResistorT<float> r1 (R);
            InductorAlphaT<float> l1 (L, (float) _fs, alpha);

            WDFSeriesT<float, ResistorT<float>, CapacitorAlphaT<float>> s1 { r1, c1 };
            WDFSeriesT<float, WDFSeriesT<float, ResistorT<float>, CapacitorAlphaT<float>>, InductorAlphaT<float>> s2 { s1, l1 };
            PolarityInverterT<float, WDFSeriesT<float, WDFSeriesT<float, ResistorT<float>, CapacitorAlphaT<float>>, InductorAlphaT<float>>> p1 { s2 };

            IdealVoltageSourceT<float, decltype (p1)> vs { p1 };

            auto a01Sine = test_utils::makeSineWave (10.0e3f, (float) _fs, 1.0f);
            processBuffer (a01Sine.getWritePointer (0), a01Sine.getNumSamples(), vs, p1, l1);
            auto a01Mag = getMagDB (a01Sine);
            expectWithinAbsoluteError (a01Mag, refMag - 1.1f, 0.1f, "Alpha = 0.1 passband gain is incorrect!");
        }
    }

    template <chowdsp::WDFT::DiodeQuality Q>
    void staticWDFTest()
    {
        using FloatType = double;
        constexpr auto Cap = (FloatType) 47.0e-9;
        constexpr auto Res = (FloatType) 4700.0;

        constexpr int num = 5;
        double data1[num] = { 1.0, 0.5, 0.0, -0.5, -1.0 };
        double data2[num] = { 1.0, 0.5, 0.0, -0.5, -1.0 };

        // dynamic
        {
            WDF::ResistiveVoltageSource<FloatType> Vs {};
            WDF::Resistor<FloatType> R1 { Res };
            auto C1 = std::make_unique<WDF::Capacitor<FloatType>> (Cap, (FloatType) _fs);

            auto S1 = std::make_unique<WDF::WDFSeries<FloatType>> (&Vs, &R1);
            auto P1 = std::make_unique<WDF::WDFParallel<FloatType>> (S1.get(), C1.get());
            auto I1 = std::make_unique<WDF::PolarityInverter<FloatType>> (P1.get());

            WDF::DiodePair<FloatType, Q> dp { I1.get(), (FloatType) 2.52e-9 };

            for (int i = 0; i < num; ++i)
            {
                Vs.setVoltage (data1[i]);
                dp.incident (P1->reflected());
                data1[i] = C1->voltage();
                P1->incident (dp.reflected());
            }
        }

        // static
        {
            ResistiveVoltageSourceT<FloatType> Vs {};
            ResistorT<FloatType> R1 { Res };
            CapacitorT<FloatType> C1 { Cap, (FloatType) _fs };

            auto S1 = makeSeries<FloatType> (Vs, R1);
            auto P1 = makeParallel<FloatType> (S1, C1);
            auto I1 = makeInverter<FloatType> (P1);

            DiodePairT<FloatType, decltype (I1), Q> dp { I1, (FloatType) 2.52e-9 };

            for (int i = 0; i < num; ++i)
            {
                Vs.setVoltage (data2[i]);
                dp.incident (P1.reflected());
                data2[i] = voltage<FloatType> (C1);
                P1.incident (dp.reflected());
            }
        }

        for (int i = 0; i < num; ++i)
            expectWithinAbsoluteError (data2[i], data1[i], (FloatType) 1.0e-6, "Static WDF is not equivalent to dynamic!");
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
            ResistorT<float> r2 { otherR };
            auto s1 = makeSeries<float> (component, r2);
            IdealCurrentSourceT<float, WDFSeriesT<float, decltype (component), ResistorT<float>>> is { s1 };
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
            ResistorT<float> { 1000.0f }, "Resistor", 1000.0f, 2000.0f, [=] (ResistorT<float>& r, float value) { r.setResistanceValue (value); }, [=] (float value) { return value; });

        // capacitor
        doImpedanceChecks (
            CapacitorT<float> { 1.0e-6f, _fs }, "Capacitor", 1.0e-6f, 2.0e-6f, [=] (CapacitorT<float>& c, float value) { c.setCapacitanceValue (value); }, [=] (float value) { return 1.0f / (2.0f * value * (float) _fs); });

        // capacitor alpha
        doImpedanceChecks (
            CapacitorAlphaT<float> { 1.0e-6f, _fs, 0.5f }, "CapacitorAlpha", 1.0e-6f, 2.0e-6f, [=] (CapacitorAlphaT<float>& c, float value) { c.setCapacitanceValue (value); }, [=] (float value) { return 1.0f / (1.5f * value * (float) _fs); });

        // inductor
        doImpedanceChecks (
            InductorT<float> { 1.0f, _fs }, "Inductor", 1.0f, 2.0f, [=] (InductorT<float>& i, float value) { i.setInductanceValue (value); }, [=] (float value) { return 2.0f * value * (float) _fs; });

        // inductor alpha
        doImpedanceChecks (
            InductorAlphaT<float> { 1.0f, _fs, 0.5f }, "InductorAlpha", 1.0f, 2.0f, [=] (InductorAlphaT<float>& i, float value) { i.setInductanceValue (value); }, [=] (float value) { return 1.5f * value * (float) _fs; });

        // resistive voltage source
        doImpedanceChecks (
            ResistiveVoltageSourceT<float> { 1000.0f }, "ResistiveVoltageSource", 1000.0f, 2000.0f, [=] (ResistiveVoltageSourceT<float>& r, float value) { r.setResistanceValue (value); }, [=] (float value) { return value; });

        // resistive current source
        doImpedanceChecks (
            ResistiveCurrentSourceT<float> { 1000.0f }, "ResistiveCurrentSource", 1000.0f, 2000.0f, [=] (ResistiveCurrentSourceT<float>& r, float value) { r.setResistanceValue (value); }, [=] (float value) { return value; });
    }

    void runTest() override
    {
        beginTest ("Voltage Divider Test");
        voltageDividerTest<float>();
        voltageDividerTest<double>();

        beginTest ("Current Divider Test");
        currentDividerTest<float>();
        currentDividerTest<double>();

        beginTest ("RC Lowpass Test");
        rcLowpassTest();

        beginTest ("Alpha Transform Test");
        alphaTransformTest();

        beginTest ("Impedance Change Test");
        impedanceChangeTest();

        beginTest ("Static WDF Test");
        staticWDFTest<chowdsp::WDFT::DiodeQuality::Good>();
        staticWDFTest<chowdsp::WDFT::DiodeQuality::Best>();
    }
};

static StaticWDFTest staticWdfTest;
