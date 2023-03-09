#pragma once

namespace chowdsp
{
/** Parameters for the waveshaper plot */
struct WaveshaperPlotParams
{
    float xMin = -5.0f;
    float xMax = 5.0f;
    float yMin = -1.1f;
    float yMax = 1.1f;
};

/** Utility class for creating waveshaper plots */
class WaveshaperPlot
{
public:
    explicit WaveshaperPlot (WaveshaperPlotParams&& plotParams);

    /** Returns the x-coordinate for a given amplitude level. */
    [[nodiscard]] float getXCoordinateForAmplitude (float amplitude) const noexcept;

    /** Returns the y-coordinate for a given amplitude level. */
    [[nodiscard]] float getYCoordinateForAmplitude (float amplitude) const noexcept;

    /** Updates the plot path internally. */
    void updatePlotPath();

    /** Returns the current plot path. */
    [[nodiscard]] const juce::Path& getPath() const noexcept { return plotPath; }

    /** Sets the size of the plot. */
    void setSize (juce::Rectangle<int> newBounds);

    /**
     * Function to use for generating the plot. Should return 2 vectors
     * (1 for the inputs signal, 1 for the output signal).
     */
    std::function<std::pair<std::vector<float>, std::vector<float>>()> generatePlotCallback {};

    WaveshaperPlotParams params;

private:
    juce::Path plotPath;

    juce::Rectangle<int> bounds;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WaveshaperPlot)
};
} // namespace chowdsp
