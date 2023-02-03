#include <CatchUtils.h>
#include <chowdsp_visualizers/chowdsp_visualizers.h>

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
        REQUIRE_MESSAGE (juce::Decibels::gainToDecibels (plot.getMagnitudeForFrequency (freq), -200.0f)
                             == Catch::Approx { expectedMag }.margin (1.0e-2f),
                         "Incorrect magnitude at frequency: " + juce::String (freq));
    }
}

const float m3DB = juce::Decibels::gainToDecibels (1.0f / juce::MathConstants<float>::sqrt2);
const float m6DB = juce::Decibels::gainToDecibels (0.5f);

TEST_CASE ("EQ Filter Plots Test", "[visualizers][EQ]")
{
    SECTION ("LPF1 Test")
    {
        testFilterPlot<chowdsp::EQ::LPF1Plot> ({ { { 1.0f, 0.0f }, { 1000.0f, m3DB }, { 8000.0f, -18.129f } } });
    }

    SECTION ("LPF2 Test")
    {
        testFilterPlot<chowdsp::EQ::LPF2Plot> ({ { { 1.0f, 0.0f }, { 1000.0f, m3DB }, { 8000.0f, -36.125f } } });
    }

    SECTION ("LPF3 Test")
    {
        testFilterPlot<chowdsp::EQ::HigherOrderLPFPlot<3>> ({ { { 1.0f, 0.0f }, { 1000.0f, m3DB }, { 8000.0f, -54.185f } } });
    }

    SECTION ("LPF8 Test")
    {
        testFilterPlot<chowdsp::EQ::HigherOrderLPFPlot<8>> ({ { { 1.0f, 0.0f }, { 1000.0f, m3DB }, { 8000.0f, -144.494f } } });
    }

    SECTION ("HPF1 Test")
    {
        testFilterPlot<chowdsp::EQ::HPF1Plot> ({ { { 50000.0f, 0.0f }, { 1000.0f, m3DB }, { 125.0f, -18.129f } } });
    }

    SECTION ("HPF2 Test")
    {
        testFilterPlot<chowdsp::EQ::HPF2Plot> ({ { { 50000.0f, 0.0f }, { 1000.0f, m3DB }, { 125.0f, -36.125f } } });
    }

    SECTION ("HPF3 Test")
    {
        testFilterPlot<chowdsp::EQ::HigherOrderHPFPlot<3>> ({ { { 50000.0f, 0.0f }, { 1000.0f, m3DB }, { 125.0f, -54.185f } } });
    }

    SECTION ("HPF8 Test")
    {
        testFilterPlot<chowdsp::EQ::HigherOrderHPFPlot<8>> ({ { { 50000.0f, 0.0f }, { 1000.0f, m3DB }, { 125.0f, -144.494f } } });
    }

    SECTION ("BPF2 Test")
    {
        testFilterPlot<chowdsp::EQ::BPF2Plot> ({ { { 125.0f, -15.053f }, { 1000.0f, 0.0f }, { 8000.0f, -15.053f } } });
    }

    SECTION ("Bell Test")
    {
        testFilterPlot<chowdsp::EQ::BellPlot> ({ { { 1.0f, 0.0f }, { 1000.0f, 10.0f }, { 100000.0f, 0.0f } }, 10.0f });
    }

    SECTION ("Notch Test")
    {
        testFilterPlot<chowdsp::EQ::NotchPlot> ({ { { 1.0f, 0.0f }, { 1000.0f, -200.0f }, { 100000.0f, 0.0f } } });
    }

    SECTION ("Low-Shelf Test")
    {
        testFilterPlot<chowdsp::EQ::LowShelfPlot> ({ { { 1.0f, 10.0f }, { 1000.0f, 5.0f }, { 100000.0f, 0.0f } }, 10.0f });
    }

    SECTION ("High-Shelf Test")
    {
        testFilterPlot<chowdsp::EQ::HighShelfPlot> ({ { { 1.0f, 0.0f }, { 1000.0f, 5.0f }, { 100000.0f, 10.0f } }, 10.0f });
    }
}
