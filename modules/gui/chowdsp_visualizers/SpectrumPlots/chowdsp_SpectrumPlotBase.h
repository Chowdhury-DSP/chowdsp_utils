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

class SpectrumPlotBase : public juce::Component
{
public:
    explicit SpectrumPlotBase (SpectrumPlotParams&& params);

    [[nodiscard]] float getXCoordinateForFrequency (float freqHz) const;
    [[nodiscard]] float getFrequencyForXCoordinate (float xCoord) const;

    [[nodiscard]] float getYCoordinateForDecibels (float magDB) const;

    void drawFrequencyLines (juce::Graphics& g, const std::vector<float>& freqHz, float lineThickness = 1.0f, std::vector<float> dashLengths = {}) const;

    void drawMagnitudeLines (juce::Graphics& g, const std::vector<float>& magDB, float lineThickness = 1.0f, std::vector<float> dashLengths = {}) const;

protected:
    SpectrumPlotParams params;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SpectrumPlotBase)
};
} // namespace chowdsp
