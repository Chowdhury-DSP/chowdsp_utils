#include <TimedUnitTest.h>
#include <chowdsp_visualizers/chowdsp_visualizers.h>

class EQFilterPlotsTest : public TimedUnitTest
{
public:
    EQFilterPlotsTest() : TimedUnitTest ("EQ Filter Plots Test") {}

    struct TestConfig
    {
        std::vector<std::pair<float, float>> testVals;
        float gainDB = 0.0f;
        float cutoffFreq = 1000.0f;
        float qVal = chowdsp::CoefficientCalculators::butterworthQ<float>;
    };

    template <typename PlotType>
    void testFilterPlot (const TestConfig& config)
    {
        PlotType plot;
        plot.setCutoffFrequency (config.cutoffFreq);
        plot.setQValue (config.qVal);
        plot.setGainDecibels (config.gainDB);

        for (auto [freq, expectedMag] : config.testVals)
        {
            expectWithinAbsoluteError (juce::Decibels::gainToDecibels (plot.getMagnitudeForFrequency (freq), -200.0f),
                                       expectedMag,
                                       1.0e-2f,
                                       "Incorrect magnitude at frequency: " + juce::String (freq));
        }
    }

    const float m3DB = juce::Decibels::gainToDecibels (1.0f / juce::MathConstants<float>::sqrt2);
    const float m6DB = juce::Decibels::gainToDecibels (0.5f);

    void runTestTimed() override
    {
        beginTest ("LPF1 Test");
        testFilterPlot<chowdsp::EQ::LPF1Plot> ({ { { 1.0f, 0.0f }, { 1000.0f, m3DB }, { 8000.0f, -18.129f } } });

        beginTest ("LPF2 Test");
        testFilterPlot<chowdsp::EQ::LPF2Plot> ({ { { 1.0f, 0.0f }, { 1000.0f, m3DB }, { 8000.0f, -36.125f } } });

        beginTest ("LPF3 Test");
        testFilterPlot<chowdsp::EQ::HigherOrderLPFPlot<3>> ({ { { 1.0f, 0.0f }, { 1000.0f, m3DB }, { 8000.0f, -54.185f } } });

        beginTest ("LPF8 Test");
        testFilterPlot<chowdsp::EQ::HigherOrderLPFPlot<8>> ({ { { 1.0f, 0.0f }, { 1000.0f, m3DB }, { 8000.0f, -144.494f } } });

        beginTest ("HPF1 Test");
        testFilterPlot<chowdsp::EQ::HPF1Plot> ({ { { 50000.0f, 0.0f }, { 1000.0f, m3DB }, { 125.0f, -18.129f } } });

        beginTest ("HPF2 Test");
        testFilterPlot<chowdsp::EQ::HPF2Plot> ({ { { 50000.0f, 0.0f }, { 1000.0f, m3DB }, { 125.0f, -36.125f } } });

        beginTest ("HPF3 Test");
        testFilterPlot<chowdsp::EQ::HigherOrderHPFPlot<3>> ({ { { 50000.0f, 0.0f }, { 1000.0f, m3DB }, { 125.0f, -54.185f } } });

        beginTest ("HPF8 Test");
        testFilterPlot<chowdsp::EQ::HigherOrderHPFPlot<8>> ({ { { 50000.0f, 0.0f }, { 1000.0f, m3DB }, { 125.0f, -144.494f } } });

        beginTest ("BPF2 Test");
        testFilterPlot<chowdsp::EQ::BPF2Plot> ({ { { 125.0f, -15.053f }, { 1000.0f, 0.0f }, { 8000.0f, -15.053f } } });

        beginTest ("Bell Test");
        testFilterPlot<chowdsp::EQ::BellPlot> ({ { { 1.0f, 0.0f }, { 1000.0f, 10.0f }, { 100000.0f, 0.0f } }, 10.0f });

        beginTest ("Notch Test");
        testFilterPlot<chowdsp::EQ::NotchPlot> ({ { { 1.0f, 0.0f }, { 1000.0f, -200.0f }, { 100000.0f, 0.0f } } });

        beginTest ("Low-Shelf Test");
        testFilterPlot<chowdsp::EQ::LowShelfPlot> ({ { { 1.0f, 10.0f }, { 1000.0f, 5.0f }, { 100000.0f, 0.0f } }, 10.0f });

        beginTest ("High-Shelf Test");
        testFilterPlot<chowdsp::EQ::HighShelfPlot> ({ { { 1.0f, 0.0f }, { 1000.0f, 5.0f }, { 100000.0f, 10.0f } }, 10.0f });
    }
};

static EQFilterPlotsTest eqFilterPlotsTest;
