#include "test_utils.h"
#include "CommonWDFTests.h"

namespace
{
constexpr double _fs = 44100.0;
} // namespace

using namespace chowdsp;

/** Unit tests for chowdsp WDF classes with SIMD support. Includes test for:
 *   - Test of the simdSignum function used internally
 *   - Test accuracy between SIMD and scalar WDF for diode clipper circuit
 *   - Test accuracy between static and run-time SIMD WDF
 */
class SIMDWDFTest : public UnitTest
{
public:
    SIMDWDFTest() : UnitTest ("SIMD Wave Digital Filter Test") {}

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
            WDF::ResistiveVoltageSource<FloatType> Vs {};
            WDF::Resistor<FloatType> R1 { Res };
            auto C1 = std::make_unique<WDF::Capacitor<FloatType>> (Cap, (FloatType) _fs);

            auto S1 = std::make_unique<WDF::WDFSeries<FloatType>> (&Vs, &R1);
            auto P1 = std::make_unique<WDF::WDFParallel<FloatType>> (S1.get(), C1.get());
            auto I1 = std::make_unique<WDF::PolarityInverter<FloatType>> (P1.get());

            WDF::DiodePair dp { I1.get(), (FloatType) 2.52e-9 };
            dp.setDiodeParameters ((FloatType) 2.52e-9, (FloatType) 0.02585, 1);

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
            WDF::ResistiveVoltageSource<VType> Vs {};
            WDF::Resistor<VType> R1 { Res };
            auto C1 = std::make_unique<WDF::Capacitor<VType>> (Cap, (VType) _fs);

            auto S1 = std::make_unique<WDF::WDFSeries<VType>> (&Vs, &R1);
            auto P1 = std::make_unique<WDF::WDFParallel<VType>> (S1.get(), C1.get());
            auto I1 = std::make_unique<WDF::PolarityInverter<VType>> (P1.get());

            WDF::DiodePair dp { I1.get(), (VType) 2.52e-9 };

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

        constexpr int num = 5;
        Vec data1[num] = { 1.0, 0.5, 0.0, -0.5, -1.0 };
        Vec data2[num] = { 1.0, 0.5, 0.0, -0.5, -1.0 };

        // dynamic
        {
            using Resistor = WDF::Resistor<Vec>;
            using Capacitor = WDF::CapacitorAlpha<Vec>;
            using ResVs = WDF::ResistiveVoltageSource<Vec>;

            ResVs Vs { 1.0e-9f };
            Resistor r162 { 4700.0f };
            Resistor r163 { 100000.0f };

            auto c40 = std::make_unique<Capacitor> ((FloatType) 0.015e-6f, (FloatType) _fs, (FloatType) 0.029f);
            auto P1 = std::make_unique<WDF::WDFParallel<Vec>> (c40.get(), &r163);
            auto S1 = std::make_unique<WDF::WDFSeries<Vec>> (&Vs, P1.get());
            auto I1 = std::make_unique<WDF::PolarityInverter<Vec>> (&r162);
            auto P2 = std::make_unique<WDF::WDFParallel<Vec>> (I1.get(), S1.get());

            WDF::Diode<Vec> d53 { P2.get(), 2.52e-9f }; // 1N4148 diode
            d53.setDiodeParameters ((FloatType) 2.52e-9, (FloatType) 0.02585, 1);

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
            using Resistor = WDFT::ResistorT<Vec>;
            using Capacitor = WDFT::CapacitorAlphaT<Vec>;
            using ResVs = WDFT::ResistiveVoltageSourceT<Vec>;

            ResVs Vs { 1.0e-9f };
            Resistor r162 { 4700.0f };
            Resistor r163 { 100000.0f };
            Capacitor c40 { (FloatType) 0.015e-6f, (FloatType) _fs, (FloatType) 0.029f };

            auto P1 = WDFT::makeParallel<Vec> (c40, r163);
            auto S1 = WDFT::makeSeries<Vec> (Vs, P1);
            auto I1 = WDFT::makeInverter<Vec> (r162);
            auto P2 = WDFT::makeParallel<Vec> (I1, S1);
            WDFT::DiodeT<Vec, decltype (P2)> d53 { P2, 2.52e-9f }; // 1N4148 diode

            for (int i = 0; i < num; ++i)
            {
                Vs.setVoltage (data2[i]);
                d53.incident (P2.reflected());
                data2[i] = WDFT::voltage<Vec> (r162);
                P2.incident (d53.reflected());
            }
        }

        for (int i = 0; i < num; ++i)
            for (size_t k = 0; k < Vec::size(); ++k)
                expectWithinAbsoluteError (data2[i].get (k), data1[i].get (k), (FloatType) 1.0e-6, "Static WDF is not equivalent to dynamic!");
    }

    void runTest() override
    {
        beginTest ("SIMD Signum Test");
        simdSignumTest();

        beginTest ("SIMD WDF Test");
        voltageDividerTest<dsp::SIMDRegister<float>> (*this);
        voltageDividerTest<dsp::SIMDRegister<double>> (*this);
        currentDividerTest<dsp::SIMDRegister<float>> (*this);
        currentDividerTest<dsp::SIMDRegister<double>> (*this);
        diodeClipperSIMDTest();

        beginTest ("Static SIMD WDF Test");
        staticSIMDWDFTest();
    }
};

static SIMDWDFTest simdWdfTest;
