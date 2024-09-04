#include "VizTestUtils.h"
#include <chowdsp_visualizers/chowdsp_visualizers.h>

struct TestWaveshapePlotter : juce::Component
{
    TestWaveshapePlotter()
        : plot ({})
    {
        plot.generatePlotCallback = [this]
        {
            static constexpr int numPoints = 1024;
            static constexpr float fs = 16000.0f;
            static constexpr float sineFreq = 100.0f;

            std::vector<float> xData, yData;
            xData.resize ((size_t) numPoints);
            yData.resize ((size_t) numPoints);

            for (auto [idx, x] : chowdsp::enumerate (xData))
                x = plot.params.xMax * std::sin (juce::MathConstants<float>::twoPi * (float) idx * sineFreq / fs);

            for (auto [idx, y] : chowdsp::enumerate (yData))
                y = std::tanh (xData[idx]);

            return std::make_pair (xData, yData);
        };
        setSize (500, 300);
    }

    void paint (juce::Graphics& g) override
    {
        g.fillAll (juce::Colours::black);

        g.setColour (juce::Colours::red);
        g.strokePath (plot.getPath(), juce::PathStrokeType { 2.0f });
    }

    void resized() override
    {
        plot.setSize (getLocalBounds());
    }

    chowdsp::WaveshaperPlot plot;
};

TEST_CASE ("Waveshaper Plot Test", "[visualizers]")
{
    TestWaveshapePlotter comp {};
    const auto testScreenshot = comp.createComponentSnapshot ({ 500, 300 });
    VizTestUtils::saveImage (testScreenshot, "waveshaper_plot.png");

    const auto refScreenshot = VizTestUtils::loadImage ("waveshaper_plot.png");
    VizTestUtils::compareImages (testScreenshot, refScreenshot);
}
