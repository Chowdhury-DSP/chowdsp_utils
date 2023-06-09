#include "chowdsp_GainComputerPlot.h"

namespace chowdsp::compressor
{
float GainComputerPlot::decibelsToYCoord (float dB) const
{
    return juce::jmap (dB, params.yMin, params.yMax, (float) getHeight(), 0.0f);
}

float GainComputerPlot::decibelsToXCoord (float dB) const
{
    return juce::jmap (dB, params.xMin, params.xMax, 0.0f, (float) getWidth());
}

void GainComputerPlot::updatePlotPath (nonstd::span<const float> inSpan, nonstd::span<const float> outSpan)
{
    plotPath.clear();
    plotPath.preallocateSpace (4 * (int) inSpan.size());

    const auto getPlotPoint = [this, &inSpan, &outSpan] (size_t pointIdx) -> juce::Point<float>
    {
        return { decibelsToXCoord (juce::Decibels::gainToDecibels (inSpan[pointIdx])),
                 decibelsToYCoord (juce::Decibels::gainToDecibels (inSpan[pointIdx] * outSpan[pointIdx])) };
    };

    plotPath.startNewSubPath (getPlotPoint (0));
    for (size_t i = 1; i < inSpan.size(); ++i)
        plotPath.lineTo (getPlotPoint (i));

    repaint();
}

void GainComputerPlot::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black);

    const auto drawHorizontalLine = [this, &g] (float yDB)
    {
        const auto yCoord = decibelsToYCoord (yDB);
        g.drawLine (0.0f, yCoord, (float) getWidth(), yCoord);
    };

    const auto drawVerticalLine = [this, &g] (float xDB)
    {
        const auto xCoord = decibelsToXCoord (xDB);
        g.drawLine (xCoord, 0.0f, xCoord, (float) getHeight());
    };

    g.setColour (juce::Colours::lightgrey.withAlpha (0.5f));
    drawHorizontalLine (-20.0f);
    drawHorizontalLine (-10.0f);
    drawHorizontalLine (0.0f);
    drawVerticalLine (-20.0f);
    drawVerticalLine (-10.0f);
    drawVerticalLine (0.0f);

    g.setColour (juce::Colours::dodgerblue);
    drawHorizontalLine (threshDB);
    drawVerticalLine (threshDB);

    g.setColour (juce::Colours::red);
    g.strokePath (plotPath, juce::PathStrokeType { 2.0f });
}
} // namespace chowdsp::compressor
