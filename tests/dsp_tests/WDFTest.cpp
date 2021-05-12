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
            expectWithinAbsoluteError (data2[i].value[0], data1[i], (FloatType) 1.0e-6, "SIMD WDF is not equivalent to float WDF!");
    }

    void runTest() override
    {
        beginTest ("Voltage Divider Test");
        voltageDividerTest<float>();
        voltageDividerTest<double>();

        beginTest ("RC Lowpass Test");
        rcLowpassTest();

        beginTest ("Static WDF Test");
        staticWDFTest();

        beginTest ("SIMD Signum Test");
        simdSignumTest();

        beginTest ("SIMD WDF Test");
        voltageDividerTest<dsp::SIMDRegister<float>>();
        voltageDividerTest<dsp::SIMDRegister<double>>();
        diodeClipperSIMDTest();
    }
};

static WDFTest wdfTest;
