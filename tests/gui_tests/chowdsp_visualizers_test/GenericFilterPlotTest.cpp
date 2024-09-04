#include "VizTestUtils.h"
#include <chowdsp_visualizers/chowdsp_visualizers.h>

TEST_CASE ("Generic Filter Plot Test", "[visualizers]")
{
    SECTION ("No Filter Test")
    {
        chowdsp::SpectrumPlotBase base {
            chowdsp::SpectrumPlotParams {
                20.0f,
                20000.0f,
                -30.0f,
                30.0f }
        };
        chowdsp::GenericFilterPlotter plotter { base, {} };
        plotter.runFilterCallback = [] (const float* in, float* out, int N)
        {
            std::copy (in, in + N, out);
        };

        const auto [freqAxis, magAxis] = plotter.plotFilterMagnitudeResponse();
        REQUIRE (freqAxis.size() == size_t (1 << plotter.params.fftOrder) / 2 + 1);
        REQUIRE (magAxis.size() == freqAxis.size());
        for (auto& mag : magAxis)
            REQUIRE (juce::approximatelyEqual (mag, 0.0f));
    }

    SECTION ("Filter Plot Test")
    {
        struct TestComponent : chowdsp::SpectrumPlotBase
        {
            TestComponent() : chowdsp::SpectrumPlotBase ({ 20.0f, 20000.0f, -30.0f, 6.0f, std::log (20000.0f / 20.0f), 36.0f }),
                              plotter (*this, {})
            {
                filter.setCutoffFrequency (1000.0f);
                filter.setQValue (0.5f);
                filter.prepare ({ plotter.params.sampleRate, uint32_t (1 << plotter.params.fftOrder), 1 });
                plotter.runFilterCallback = [this] (const float* in, float* out, int N)
                {
                    std::copy (in, in + N, out);
                    filter.processBlock (chowdsp::BufferView { out, N });
                };
                setSize (500, 300);
                plotter.updateFilterPlot();
            }

            void paint (juce::Graphics& g) override
            {
                g.fillAll (juce::Colours::black);

                g.setColour (juce::Colours::red);
                g.strokePath (plotter.getPath(), juce::PathStrokeType { 1.5f });
            }

            chowdsp::GenericFilterPlotter plotter;
            chowdsp::SVFNotch<float> filter;
        };

        TestComponent comp {};
        const auto testScreenshot = comp.createComponentSnapshot ({ 500, 300 });
        // VizTestUtils::saveImage (testScreenshot, "generic_filter_plot.png");

        const auto refScreenshot = VizTestUtils::loadImage ("generic_filter_plot.png");
        VizTestUtils::compareImages (testScreenshot, refScreenshot);
    }
}
