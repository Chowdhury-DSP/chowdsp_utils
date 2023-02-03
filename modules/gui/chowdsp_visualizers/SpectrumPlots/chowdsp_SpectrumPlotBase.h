#pragma once

namespace chowdsp
{
/** Basic parameters for a frequency spectrum plot. */
struct SpectrumPlotParams
{
    const float minFrequencyHz = 20.0f; // the lowest frequency in the plot
    const float maxFrequencyHz = 20000.0f; // the highest frequency in the plot

    const float minMagnitudeDB = -30.0f;
    const float maxMagnitudeDB = 30.0f;

    const float frequencyScale = std::log (maxFrequencyHz / minFrequencyHz);
    const float rangeDB = maxMagnitudeDB - minMagnitudeDB;
};

/** Base class for frequency domain plots. */
class SpectrumPlotBase : public juce::Component
{
public:
    explicit SpectrumPlotBase (SpectrumPlotParams&& params);

    /** Returns the x-coordinate for the given frequency (in Hertz). */
    [[nodiscard]] float getXCoordinateForFrequency (float freqHz) const;

    /** Returns a frequency (in Hertz) for a given x-coordinate. */
    [[nodiscard]] float getFrequencyForXCoordinate (float xCoord) const;

    /** Returns the y-coordinate for the given magnitude (in Decibels). */
    [[nodiscard]] float getYCoordinateForDecibels (float magDB) const;

    /** Draws a set of frequency grid lines. */
    template <typename FreqsContainer = std::initializer_list<float>, typename Dashes = std::initializer_list<float>>
    void drawFrequencyLines (const juce::Graphics& g, const FreqsContainer& freqHzList, float lineThickness = 1.0f, const Dashes& dashLengths = {}) const
    {
        for (auto& freqHz : freqHzList)
        {
            const auto line = juce::Line<float>::fromStartAndAngle (juce::Point { getXCoordinateForFrequency (freqHz), 0.0f }, (float) getHeight(), juce::MathConstants<float>::pi);
            if (std::size (dashLengths) == 0)
                g.drawLine (line, lineThickness);
            else
                g.drawDashedLine (line, std::data (dashLengths), (int) std::size (dashLengths), lineThickness);
        }
    }

    /** Draws a set of magnitude grid lines. */
    template <typename MagsContainer = std::initializer_list<float>, typename Dashes = std::initializer_list<float>>
    void drawMagnitudeLines (const juce::Graphics& g, const MagsContainer& magDBList, float lineThickness = 1.0f, const Dashes& dashLengths = {}) const
    {
        for (auto& magDB : magDBList)
        {
            const auto line = juce::Line<float>::fromStartAndAngle (juce::Point { 0.0f, getYCoordinateForDecibels (magDB) }, (float) getWidth(), juce::MathConstants<float>::halfPi);
            if (std::size (dashLengths) == 0)
                g.drawLine (line, lineThickness);
            else
                g.drawDashedLine (line, std::data (dashLengths), (int) std::size (dashLengths), lineThickness);
        }
    }

    const SpectrumPlotParams params;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SpectrumPlotBase)
};
} // namespace chowdsp
