#pragma once

namespace chowdsp::compressor
{
/** A waveform view that can be used to visualize level detector data */
class LevelDetectorVisualizer : public WaveformView<2>
{
public:
    using WaveformView<2>::WaveformView;

    double secondsToVisualize = 2.0;

    juce::Colour audioColour = juce::Colours::dodgerblue;
    juce::Colour levelColour = juce::Colours::red;

    struct Params
    {
        float yMin = -1.0f;
        float yMax = 1.0f;
    };
    Params params;

protected:
    void paintChannel (int channelIndex,
                       juce::Graphics& g,
                       juce::Rectangle<float> area,
                       const juce::Range<float>* levels,
                       int numLevels,
                       int nextSample) override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LevelDetectorVisualizer)
};
} // namespace chowdsp::compressor
