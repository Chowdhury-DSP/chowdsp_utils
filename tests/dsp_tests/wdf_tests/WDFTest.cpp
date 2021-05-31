#include "test_utils.h"
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

    template <typename FloatType>
    void voltageDividerTest()
    {
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

        if (vOut != (FloatType) 5.0)
            expect (false, "Voltage divider: incorrect voltage!");
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
            Capacitor<float> c1 (C, (float) fs);
            Resistor<float> r1 (R);
            Inductor<float> l1 (L, (float) fs);

            WDFSeries<float> s1 (&r1, &c1);
            WDFSeries<float> s2 (&s1, &l1);
            PolarityInverter<float> p1 (&s2);

            IdealVoltageSource<float> vs;
            vs.connectToNode (&p1);

            auto refSine = test_utils::makeSineWave (10.0e3f, (float) fs, 1.0f);
            processBuffer (refSine.getWritePointer (0), refSine.getNumSamples(), vs, p1, l1);
            refMag = getMagDB (refSine);
            expectWithinAbsoluteError (refMag, 0.0f, 0.1f, "Reference highpass passband gain is incorrect!");
        }

        // alpha = 1.0 filter
        {
            constexpr float alpha = 1.0f;
            CapacitorAlpha<float> c1 (C, (float) fs, alpha);
            Resistor<float> r1 (R);
            InductorAlpha<float> l1 (L, (float) fs, alpha);

            WDFSeries<float> s1 (&r1, &c1);
            WDFSeries<float> s2 (&s1, &l1);
            PolarityInverter<float> p1 (&s2);

            IdealVoltageSource<float> vs;
            vs.connectToNode (&p1);

            auto a1Sine = test_utils::makeSineWave (10.0e3f, (float) fs, 1.0f);
            processBuffer (a1Sine.getWritePointer (0), a1Sine.getNumSamples(), vs, p1, l1);
            auto a1Mag = getMagDB (a1Sine);
            expectWithinAbsoluteError (a1Mag, 0.0f, 0.1f, "Alpha = 1 highpass passband gain is incorrect!");
            expectWithinAbsoluteError (a1Mag, refMag, 1.0e-6f, "Alpha = 1 passband gain is not equal to reference!");
        }

        // alpha = 0.1 filter
        {
            constexpr float alpha = 0.1f;
            CapacitorAlpha<float> c1 (C, (float) fs, alpha);
            Resistor<float> r1 (R);
            InductorAlpha<float> l1 (L, (float) fs, alpha);

            WDFSeries<float> s1 (&r1, &c1);
            WDFSeries<float> s2 (&s1, &l1);
            PolarityInverter<float> p1 (&s2);

            IdealVoltageSource<float> vs;
            vs.connectToNode (&p1);

            auto a01Sine = test_utils::makeSineWave (10.0e3f, (float) fs, 1.0f);
            processBuffer (a01Sine.getWritePointer (0), a01Sine.getNumSamples(), vs, p1, l1);
            auto a01Mag = getMagDB (a01Sine);
            expectWithinAbsoluteError (a01Mag, refMag - 1.1f, 0.1f, "Alpha = 0.1 passband gain is incorrect!");
        }
    }

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
            ResistiveVoltageSource<FloatType> Vs {};
            Resistor<FloatType> R1 { Res };
            auto C1 = std::make_unique<Capacitor<FloatType>> (Cap, (FloatType) fs);

            auto S1 = std::make_unique<WDFSeries<FloatType>> (&Vs, &R1);
            auto P1 = std::make_unique<WDFParallel<FloatType>> (S1.get(), C1.get());
            auto I1 = std::make_unique<PolarityInverter<FloatType>> (P1.get());

            DiodePair dp { (FloatType) 2.52e-9, (FloatType) 0.02585 };
            dp.connectToNode (I1.get());

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
            ResistiveVoltageSource<FloatType> Vs {};
            Resistor<FloatType> R1 { Res };
            Capacitor<FloatType> C1 { Cap, (FloatType) fs };

            auto S1 = makeSeries<FloatType> (Vs, R1);
            auto P1 = makeParallel<FloatType> (S1, C1);
            auto I1 = makeInverter<FloatType> (P1);

            DiodePair dp { (FloatType) 2.52e-9, (FloatType) 0.02585 };
            dp.connectToNode (&I1);

            for (int i = 0; i < num; ++i)
            {
                Vs.setVoltage (data2[i]);
                dp.incident (P1.reflected());
                data2[i] = C1.voltage();
                P1.incident (dp.reflected());
            }
        }

        for (int i = 0; i < num; ++i)
            expectWithinAbsoluteError (data2[i], data1[i], (FloatType) 1.0e-6, "Static WDF is not equivalent to dynamic!");
    }

    void simdSignumTest()
    {
        dsp::SIMDRegister<float> testReg;
        testReg.set (0, -1.0f);
        testReg.set (1, 0.0f);
        testReg.set (2, 0.5f);
        testReg.set (3, 1.0f);

        auto signumReg = signumSIMD (testReg);

        for (size_t i = 0; i < testReg.size(); ++i)
            expectEquals (signumReg.get (i), (float) signum (testReg.get (i)), "SIMD Signum is incorrect!");
    }

    void diodeClipperSIMDTest()
    {
        using FloatType = double;
        using VType = dsp::SIMDRegister<FloatType>;
        constexpr auto Cap = (FloatType) 47.0e-9;
        constexpr auto Res = (FloatType) 4700.0;

        constexpr int num = 5;
        FloatType data1[num] = { 1.0, 0.5, 0.0, -0.5, -1.0 };
        VType data2[num] = { 1.0, 0.5, 0.0, -0.5, -1.0 };

        // Normal
        {
            ResistiveVoltageSource<FloatType> Vs {};
            Resistor<FloatType> R1 { Res };
            auto C1 = std::make_unique<Capacitor<FloatType>> (Cap, (FloatType) fs);

            auto S1 = std::make_unique<WDFSeries<FloatType>> (&Vs, &R1);
            auto P1 = std::make_unique<WDFParallel<FloatType>> (S1.get(), C1.get());
            auto I1 = std::make_unique<PolarityInverter<FloatType>> (P1.get());

            DiodePair dp { (FloatType) 2.52e-9, (FloatType) 0.02585 };
            dp.connectToNode (I1.get());

            for (int i = 0; i < num; ++i)
            {
                Vs.setVoltage (data1[i]);
                dp.incident (P1->reflected());
                data1[i] = C1->voltage();
                P1->incident (dp.reflected());
            }
        }

        // SIMD
        {
            ResistiveVoltageSource<VType> Vs {};
            Resistor<VType> R1 { Res };
            auto C1 = std::make_unique<Capacitor<VType>> (Cap, (VType) fs);

            auto S1 = std::make_unique<WDFSeries<VType>> (&Vs, &R1);
            auto P1 = std::make_unique<WDFParallel<VType>> (S1.get(), C1.get());
            auto I1 = std::make_unique<PolarityInverter<VType>> (P1.get());

            DiodePair dp { (VType) 2.52e-9, (VType) 0.02585 };
            dp.connectToNode (I1.get());

            for (int i = 0; i < num; ++i)
            {
                Vs.setVoltage (data2[i]);
                dp.incident (P1->reflected());
                data2[i] = C1->voltage();
                P1->incident (dp.reflected());
            }
        }

        for (int i = 0; i < num; ++i)
            expectWithinAbsoluteError (data2[i].get (0), data1[i], (FloatType) 1.0e-6, "SIMD WDF is not equivalent to float WDF!");
    }

    void staticSIMDWDFTest()
    {
        using FloatType = float;
        using Vec = dsp::SIMDRegister<FloatType>;

        using Resistor = Resistor<Vec>;
        using Capacitor = CapacitorAlpha<Vec>;
        using ResVs = ResistiveVoltageSource<Vec>;

        constexpr int num = 5;
        Vec data1[num] = { 1.0, 0.5, 0.0, -0.5, -1.0 };
        Vec data2[num] = { 1.0, 0.5, 0.0, -0.5, -1.0 };

        // dynamic
        {
            ResVs Vs { 1.0e-9f };
            Resistor r162 { 4700.0f };
            Resistor r163 { 100000.0f };
            Diode<Vec> d53 { 2.52e-9f, 25.85e-3f }; // 1N4148 diode

            auto c40 = std::make_unique<Capacitor> ((FloatType) 0.015e-6f, (FloatType) fs, (FloatType) 0.029f);
            auto P1 = std::make_unique<WDFParallel<Vec>> (c40.get(), &r163);
            auto S1 = std::make_unique<WDFSeries<Vec>> (&Vs, P1.get());
            auto I1 = std::make_unique<PolarityInverter<Vec>> (&r162);
            auto P2 = std::make_unique<WDFParallel<Vec>> (I1.get(), S1.get());

            d53.connectToNode (P2.get());

            for (int i = 0; i < num; ++i)
            {
                Vs.setVoltage (data1[i]);
                d53.incident (P2->reflected());
                data1[i] = r162.voltage();
                P2->incident (d53.reflected());
            }
        }

        // static
        {
            ResVs Vs { 1.0e-9f };
            Resistor r162 { 4700.0f };
            Resistor r163 { 100000.0f };
            Capacitor c40 { (FloatType) 0.015e-6f, (FloatType) fs, (FloatType) 0.029f };
            Diode<Vec> d53 { 2.52e-9f, 25.85e-3f }; // 1N4148 diode

            auto P1 = makeParallel<Vec> (c40, r163);
            auto S1 = makeSeries<Vec> (Vs, P1);
            auto I1 = makeInverter<Vec> (r162);
            auto P2 = makeParallel<Vec> (I1, S1);

            d53.connectToNode (&P2);

            for (int i = 0; i < num; ++i)
            {
                Vs.setVoltage (data2[i]);
                d53.incident (P2.reflected());
                data2[i] = r162.voltage();
                P2.incident (d53.reflected());
            }
        }

        for (int i = 0; i < num; ++i)
            for (size_t k = 0; k < Vec::size(); ++k)
                expectWithinAbsoluteError (data2[i].get (k), data1[i].get (k), (FloatType) 1.0e-6, "Static WDF is not equivalent to dynamic!");
    }

    void runTest() override
    {
        beginTest ("Voltage Divider Test");
        voltageDividerTest<float>();
        voltageDividerTest<double>();

        beginTest ("RC Lowpass Test");
        rcLowpassTest();

        beginTest ("Alpha Transform Test");
        alphaTransformTest();

        beginTest ("Static WDF Test");
        staticWDFTest();

        beginTest ("SIMD Signum Test");
        simdSignumTest();

        beginTest ("SIMD WDF Test");
        voltageDividerTest<dsp::SIMDRegister<float>>();
        voltageDividerTest<dsp::SIMDRegister<double>>();
        diodeClipperSIMDTest();

        beginTest ("Static SIMD WDF Test");
        staticSIMDWDFTest();
    }
};

static WDFTest wdfTest;
