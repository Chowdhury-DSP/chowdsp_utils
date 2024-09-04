#include "VizTestUtils.h"

#include <chowdsp_compressor/chowdsp_compressor.h>
#include <chowdsp_visualizers/chowdsp_visualizers.h>

TEST_CASE ("Gain Computer Plot Test", "[visualizers][compressor]")
{
    static constexpr int width = 500;
    static constexpr int height = 500;
    static constexpr int plotNumSamples = 128;

    chowdsp::compressor::GainComputerPlot plot;
    plot.setSize (width, height);
    plot.setThreshold (-12.0f);

    chowdsp::compressor::GainComputer<float, chowdsp::compressor::FeedForwardCompGainComputer<float>> gainComputer;

    gainComputer.prepare (48000.0, plotNumSamples);
    gainComputer.setThreshold (-12.0f);
    gainComputer.setRatio (4.0f);
    gainComputer.setKnee (18.0f);
    gainComputer.reset();

    chowdsp::StaticBuffer<float, 1, plotNumSamples> inputBuffer { 1, plotNumSamples };
    chowdsp::StaticBuffer<float, 1, plotNumSamples> outputBuffer { 1, plotNumSamples };

    for (auto [n, sample] : chowdsp::enumerate (inputBuffer.getWriteSpan (0)))
        sample = juce::Decibels::decibelsToGain (juce::jmap ((float) n,
                                                             0.0f,
                                                             (float) inputBuffer.getNumSamples(),
                                                             plot.params.xMin,
                                                             plot.params.xMax));

    gainComputer.processBlock (inputBuffer, outputBuffer);
    plot.updatePlotPath (inputBuffer.getReadSpan (0), outputBuffer.getReadSpan (0));

    const auto testScreenshot = plot.createComponentSnapshot ({ width, height });
    // VizTestUtils::saveImage (testScreenshot, "gain_computer_plot.png");

    const auto refScreenshot = VizTestUtils::loadImage ("gain_computer_plot.png");
    VizTestUtils::compareImages (testScreenshot, refScreenshot);
}
