#include "../test_utils.h"

using namespace chowdsp::WDF;

namespace
{
    constexpr float fs = 48000.0f;
    constexpr float timeSeconds = 1000.0f;
}

class WDFPerfTest : public UnitTest
{
public:
    WDFPerfTest() : UnitTest ("WDFPerfTest")
    {
    }

    template <typename IIRType, typename WDFType, typename WDFTType>
    void runPerf (IIRType& iir, WDFType& wdf, WDFTType& wdfT)
    {
        auto buffer = test_utils::makeNoise (fs, timeSeconds);
        auto* x = buffer.getReadPointer (0);

        Time time;
        auto timeProcess = [&] (auto& proc)
        {
            auto start = time.getMillisecondCounterHiRes();
            for (int i = 0; i < buffer.getNumSamples(); ++i)
                proc.processSample (x[i]);
            return (time.getMillisecondCounterHiRes() - start) / 1000.0;
        };

        std::cout << "Running IIR..." << std::endl;
        double iirDuration = timeProcess (iir);

        std::cout << "Running WDF..." << std::endl;
        double wdfDuration = timeProcess (wdf);

        std::cout << "Running WDFT..." << std::endl;
        double wdfTDuration = timeProcess (wdfT);

        std::cout << "IIR: processed " << timeSeconds << " seconds of audio in " << iirDuration << " seconds" << std::endl;
        std::cout << "WDF: processed " << timeSeconds << " seconds of audio in " << wdfDuration << " seconds" << std::endl;
        std::cout << "WDFT: processed " << timeSeconds << " seconds of audio in " << wdfTDuration << " seconds" << std::endl;

        std::cout << "WDF is " << iirDuration / wdfDuration << "x faster than IIR" << std::endl;
        std::cout << "WDFT is " << iirDuration / wdfTDuration << "x faster than IIR" << std::endl;
        std::cout << "WDFT is " << wdfDuration / wdfTDuration << "x faster than WDF" << std::endl;

        expectGreaterOrEqual (wdfDuration / wdfTDuration, 1.0, "Templated WDF is slower than polymorphic!");
    }

    void firstOrderTest()
    {
        chowdsp::IIRFilter<1, float> iir;
        iir.setCoefs ({ 1.0f, 0.5f }, { 1.0f, -0.02f });

        class WDF1
        {
        public:
            WDF1()
            {
                c1 = std::make_unique<Capacitor<float>> (1.0e-6f, fs);
                s1 = std::make_unique<WDFSeries<float>> (c1.get(), &r1);
                p1 = std::make_unique<PolarityInverter<float>> (s1.get());
                vs.connectToNode (p1.get());
            }

            inline float processSample (float x)
            {
                vs.setVoltage (x);

                vs.incident (p1->reflected());
                p1->incident (vs.reflected());

                return c1->voltage();
            }

        private:
            std::unique_ptr<Capacitor<float>> c1;
            Resistor<float> r1 { 10.0e3f };

            std::unique_ptr<WDFSeries<float>> s1;
            std::unique_ptr<PolarityInverter<float>> p1;

            IdealVoltageSource<float> vs;
        } wdf;

        class WDFT1
        {
        public:
            WDFT1()
            {
                vs.connectToNode (&p1);
            }

            inline float processSample (float x)
            {
                vs.setVoltage (x);

                vs.incident (p1.reflected());
                p1.incident (vs.reflected());

                return c1.voltage();
            }

        private:
            Capacitor<float> c1 { 1.0e-6f, fs };
            Resistor<float> r1 { 10.0e3f };

            WDFSeriesT<float, Capacitor<float>, Resistor<float>> s1 { c1, r1 };
            PolarityInverterT<float, WDFSeriesT<float, Capacitor<float>, Resistor<float>>> p1 { s1 };

            IdealVoltageSource<float> vs;
        } wdfT;

        runPerf (iir, wdf, wdfT);
    }

    void secondOrderTest()
    {
        chowdsp::IIRFilter<2, float> iir;
        iir.setCoefs ({ 1.0f, 0.5f, -0.1f }, { 1.0f, -0.02f, 0.15f });

        class WDF2
        {
        public:
            WDF2()
            {
                c1 = std::make_unique<Capacitor<float>> (1.0e-6f, fs);
                s1 = std::make_unique<WDFSeries<float>> (c1.get(), &r1);

                c2 = std::make_unique<Capacitor<float>> (1.0e-6f, fs);
                p1 = std::make_unique<WDFParallel<float>> (c2.get(), s1.get());
                s2 = std::make_unique<WDFSeries<float>> (p1.get(), &r2);

                i1 = std::make_unique<PolarityInverter<float>> (s2.get());
                vs.connectToNode (i1.get());
            }

            inline float processSample (float x)
            {
                vs.setVoltage (x);

                vs.incident (i1->reflected());
                i1->incident (vs.reflected());

                return c1->voltage();
            }

        private:
            std::unique_ptr<Capacitor<float>> c1;
            std::unique_ptr<Capacitor<float>> c2;
            Resistor<float> r1 { 10.0e3f };
            Resistor<float> r2 { 10.0e3f };

            std::unique_ptr<WDFSeries<float>> s1;
            std::unique_ptr<WDFSeries<float>> s2;
            std::unique_ptr<WDFParallel<float>> p1;
            std::unique_ptr<PolarityInverter<float>> i1;

            IdealVoltageSource<float> vs;
        } wdf;

        class WDFT2
        {
        public:
            WDFT2()
            {
                vs.connectToNode (&i1);
            }

            inline float processSample (float x)
            {
                vs.setVoltage (x);

                vs.incident (i1.reflected());
                i1.incident (vs.reflected());

                return c1.voltage();
            }

        private:
            Capacitor<float> c1 { 1.0e-6f, fs };
            Capacitor<float> c2 { 1.0e-6f, fs };
            Resistor<float> r1 { 10.0e3f };
            Resistor<float> r2 { 10.0e3f };

            using S1Type = WDFSeriesT<float, Capacitor<float>, Resistor<float>>;
            S1Type s1 { c1, r1 };

            using P1Type = WDFParallelT<float, Capacitor<float>, S1Type>;
            P1Type p1 { c2, s1 };

            using S2Type = WDFSeriesT<float, Resistor<float>, P1Type>;
            S2Type s2 { r2, p1 };

            PolarityInverterT<float, S2Type> i1 { s2 };
            IdealVoltageSource<float> vs;
        } wdfT;

        runPerf (iir, wdf, wdfT);
    }

    void runTest() override
    {
        beginTest ("First-order filter test");
        firstOrderTest();

        beginTest ("Second-order filter test");
        secondOrderTest();
    }
};

static WDFPerfTest wdfPerf;
