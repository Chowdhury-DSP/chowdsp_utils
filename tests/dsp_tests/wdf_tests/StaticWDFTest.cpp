#include "test_utils.h"

namespace
{
constexpr double fs = 44100.0;
constexpr double fc = 500.0;
constexpr float error = 0.1f;
} // namespace

using namespace chowdsp::WDF;

class StaticWDFTest : public UnitTest
{
public:
    StaticWDFTest() : UnitTest ("Static Wave Digital Filter Test") {}

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

    void runTest() override
    {
        // beginTest ("Voltage Divider Test");
        // voltageDividerTest<float>();
        // voltageDividerTest<double>();

        // beginTest ("RC Lowpass Test");
        // rcLowpassTest();

        // beginTest ("Alpha Transform Test");
        // alphaTransformTest();

        beginTest ("Static WDF Test");
        staticWDFTest();
    }
};

static StaticWDFTest staticWdfTest;
