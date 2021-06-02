#include "test_utils.h"

using namespace chowdsp;

namespace
{
constexpr float _fs = 48000.0f;
#if JUCE_DEBUG
constexpr float timeSeconds = 10.0f;
#else
constexpr float timeSeconds = 1000.0f;
#endif
} // namespace

class WDFPerfTest : public UnitTest
{
public:
    WDFPerfTest() : UnitTest ("WDFPerfTest")
    {
    }

    template <typename IIRType, typename WDFType, typename WDFTType>
    void runPerf (IIRType& iir, WDFType& wdf, WDFTType& wdfT)
    {
        auto buffer = test_utils::makeNoise (_fs, timeSeconds);
        auto* x = buffer.getReadPointer (0);

        Time time;
        auto timeProcess = [&] (auto& proc) {
            auto start = time.getMillisecondCounterHiRes();
            float output = 0.0f;
            for (int i = 0; i < buffer.getNumSamples(); ++i)
                output = proc.processSample (x[i]);
            auto duration = (time.getMillisecondCounterHiRes() - start) / 1000.0;
            std::cout << "Final output: " << output << std::endl;
            return duration;
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

        expectGreaterOrEqual (wdfDuration / wdfTDuration, 0.95, "Templated WDF is slower than polymorphic!");
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
                c1 = std::make_unique<WDF::Capacitor<float>> (1.0e-6f, _fs);
                s1 = std::make_unique<WDF::WDFSeries<float>> (c1.get(), &r1);
                p1 = std::make_unique<WDF::PolarityInverter<float>> (s1.get());
                vs = std::make_unique<WDF::IdealVoltageSource<float>> (p1.get());
            }

            inline float processSample (float x)
            {
                vs->setVoltage (x);

                vs->incident (p1->reflected());
                p1->incident (vs->reflected());

                return c1->voltage();
            }

        private:
            std::unique_ptr<WDF::Capacitor<float>> c1;
            WDF::Resistor<float> r1 { 10.0e3f };

            std::unique_ptr<WDF::WDFSeries<float>> s1;
            std::unique_ptr<WDF::PolarityInverter<float>> p1;

            std::unique_ptr<WDF::IdealVoltageSource<float>> vs;
        } wdf;

        class WDFT1
        {
        public:
            WDFT1() = default;

            inline float processSample (float x)
            {
                vs.setVoltage (x);

                vs.incident (p1.reflected());
                p1.incident (vs.reflected());

                return WDFT::voltage<float> (c1);
            }

        private:
            WDFT::CapacitorT<float> c1 { 1.0e-6f, _fs };
            WDFT::ResistorT<float> r1 { 10.0e3f };

            WDFT::WDFSeriesT<float, WDFT::CapacitorT<float>, WDFT::ResistorT<float>> s1 { c1, r1 };
            WDFT::PolarityInverterT<float, WDFT::WDFSeriesT<float, WDFT::CapacitorT<float>, WDFT::ResistorT<float>>> p1 { s1 };

            WDFT::IdealVoltageSourceT<float, decltype (p1)> vs { p1 };
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
                c1 = std::make_unique<WDF::Capacitor<float>> (1.0e-6f, _fs);
                s1 = std::make_unique<WDF::WDFSeries<float>> (c1.get(), &r1);

                c2 = std::make_unique<WDF::Capacitor<float>> (1.0e-6f, _fs);
                p1 = std::make_unique<WDF::WDFParallel<float>> (c2.get(), s1.get());
                s2 = std::make_unique<WDF::WDFSeries<float>> (p1.get(), &r2);

                i1 = std::make_unique<WDF::PolarityInverter<float>> (s2.get());
                vs = std::make_unique<WDF::IdealVoltageSource<float>> (i1.get());
            }

            inline float processSample (float x)
            {
                vs->setVoltage (x);

                vs->incident (i1->reflected());
                i1->incident (vs->reflected());

                return c1->voltage();
            }

        private:
            std::unique_ptr<WDF::Capacitor<float>> c1;
            std::unique_ptr<WDF::Capacitor<float>> c2;
            WDF::Resistor<float> r1 { 10.0e3f };
            WDF::Resistor<float> r2 { 10.0e3f };

            std::unique_ptr<WDF::WDFSeries<float>> s1;
            std::unique_ptr<WDF::WDFSeries<float>> s2;
            std::unique_ptr<WDF::WDFParallel<float>> p1;
            std::unique_ptr<WDF::PolarityInverter<float>> i1;

            std::unique_ptr<WDF::IdealVoltageSource<float>> vs;
        } wdf;

        class WDFT2
        {
        public:
            WDFT2() = default;

            inline float processSample (float x)
            {
                vs.setVoltage (x);

                vs.incident (i1.reflected());
                i1.incident (vs.reflected());

                return WDFT::voltage<float> (c1);
            }

        private:
            WDFT::CapacitorT<float> c1 { 1.0e-6f, _fs };
            WDFT::CapacitorT<float> c2 { 1.0e-6f, _fs };
            WDFT::ResistorT<float> r1 { 10.0e3f };
            WDFT::ResistorT<float> r2 { 10.0e3f };

            using S1Type = WDFT::WDFSeriesT<float, WDFT::CapacitorT<float>, WDFT::ResistorT<float>>;
            S1Type s1 { c1, r1 };

            using P1Type = WDFT::WDFParallelT<float, WDFT::CapacitorT<float>, S1Type>;
            P1Type p1 { c2, s1 };

            using S2Type = WDFT::WDFSeriesT<float, WDFT::ResistorT<float>, P1Type>;
            S2Type s2 { r2, p1 };

            WDFT::PolarityInverterT<float, S2Type> i1 { s2 };
            WDFT::IdealVoltageSourceT<float, WDFT::PolarityInverterT<float, S2Type>> vs { i1 };
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
