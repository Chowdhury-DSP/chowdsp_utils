#pragma once

namespace chowdsp::compressor
{
/** Plot's the Decibel response of a gain computer */
class GainComputerPlot : public juce::Component
{
public:
    GainComputerPlot() = default;

    /** Converts a Decibel value to a x-coordinate */
    [[nodiscard]] float decibelsToXCoord (float dB) const;

    /** Converts a Decibel value to a y-coordinate */
    [[nodiscard]] float decibelsToYCoord (float dB) const;

    /** Sets the gain computer threshold in Decibels */
    void setThreshold (float newThreshDB) { threshDB = newThreshDB; }

    /** Updates the plot path with new level input/output data */
    void updatePlotPath (nonstd::span<const float> inSpan, nonstd::span<const float> outSpan);

    struct Params
    {
        float xMin = -30.0f;
        float xMax = 6.0f;
        float yMin = -30.0f;
        float yMax = 6.0f;
    };
    Params params;

protected:
    void paint (juce::Graphics& g) override;

    float threshDB = 0.0f;
    juce::Path plotPath {};

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GainComputerPlot)
};
} // namespace chowdsp::compressor
