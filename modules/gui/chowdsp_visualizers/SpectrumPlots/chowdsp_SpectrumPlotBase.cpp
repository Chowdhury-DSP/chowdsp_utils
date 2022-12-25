#include "chowdsp_SpectrumPlotBase.h"

namespace chowdsp
{
SpectrumPlotBase::SpectrumPlotBase (SpectrumPlotParams&& plotParams) : params (std::move (plotParams))
{
}

float SpectrumPlotBase::getXCoordinateForFrequency (float freqHz) const
{
    const auto xNorm = std::log (freqHz / params.minFrequencyHz) / params.frequencyScale;
    return xNorm * (float) getWidth();
}

float SpectrumPlotBase::getFrequencyForXCoordinate (float xCoord) const
{
    const auto xNorm = xCoord / (float) getWidth();
    return std::exp (xNorm * params.frequencyScale) * params.minFrequencyHz;
}

float SpectrumPlotBase::getYCoordinateForDecibels (float gainDB) const
{
    return (float) getHeight() * (params.maxMagnitudeDB - gainDB) / params.rangeDB;
}

void SpectrumPlotBase::drawFrequencyLines (const juce::Graphics& g, const std::vector<float>& freqHzList, float lineThickness, std::vector<float> dashLengths) const
{
    for (auto& freqHz : freqHzList)
    {
        const auto line = juce::Line<float>::fromStartAndAngle (juce::Point { getXCoordinateForFrequency (freqHz), 0.0f }, (float) getHeight(), juce::MathConstants<float>::pi);
        if (dashLengths.empty())
            g.drawLine (line, lineThickness);
        else
            g.drawDashedLine (line, dashLengths.data(), (int) dashLengths.size(), lineThickness);
    }
}

void SpectrumPlotBase::drawMagnitudeLines (const juce::Graphics& g, const std::vector<float>& magDBList, float lineThickness, std::vector<float> dashLengths) const
{
    for (auto& magDB : magDBList)
    {
        const auto line = juce::Line<float>::fromStartAndAngle (juce::Point { 0.0f, getYCoordinateForDecibels (magDB) }, (float) getWidth(), juce::MathConstants<float>::halfPi);
        if (dashLengths.empty())
            g.drawLine (line, lineThickness);
        else
            g.drawDashedLine (line, dashLengths.data(), (int) dashLengths.size(), lineThickness);
    }
}
} // namespace chowdsp
