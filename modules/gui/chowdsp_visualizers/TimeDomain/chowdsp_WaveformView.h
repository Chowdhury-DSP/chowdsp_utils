#pragma once

namespace chowdsp
{
/** Waveform viewer based loosely on juce::AudioVisualizerComponent */
template <int numChannels>
class WaveformView : public juce::Component,
                     public juce::Timer
{
public:
    WaveformView();

    /**
     * Changes the number of samples that the visualiser keeps in its history.
     * Note that this value refers to the number of averaged sample blocks, and each
     * block is calculated as the peak of a number of incoming audio samples. To set
     * the number of incoming samples per block, use setSamplesPerBlock().
     */
    void setBufferSize (int bufferSize);

    /** Prepare the visualizer for an incoming buffer size. */
    void setSamplesPerBlock (int newNumInputSamplesPerBlock) noexcept;

    /** Returns the prepared buffer size. */
    [[nodiscard]] int getSamplesPerBlock() const noexcept { return inputSamplesPerBlock; }

    /** Clears the contents of the buffers. */
    void clear();

    /**
     * Pushes a buffer of channels data.
     * The number of channels provided here is expected to match the number of channels
     * that this AudioVisualiserComponent has been told to use.
    */
    template <typename SampleType>
    void pushBuffer (const BufferView<const SampleType>& buffer) noexcept;

    template <typename SampleType>
    void pushChannel (int channelIndex, const nonstd::span<const SampleType>& channelData) noexcept;

    void paint (juce::Graphics& g) override;
    virtual void paintChannel (int channelIndex, juce::Graphics&, juce::Rectangle<float> bounds, const juce::Range<float>* levels, int numLevels, int nextSample);
    void visibilityChanged() override;
    juce::Colour backgroundColour = juce::Colours::whitesmoke;

protected:
    void timerCallback() override { repaint(); }

private:
    struct ChannelInfo
    {
        ChannelInfo (WaveformView& owner, int bufferSize);
        void clear() noexcept;
        template <typename T>
        void pushSamples (const T* inputSamples, int num) noexcept;
        void pushSample (float newSample) noexcept;
        void setBufferSize (int newSize);

        WaveformView& owner;
        std::vector<juce::Range<float>> levels;
        juce::Range<float> value;
        std::atomic<int> nextSample { 0 }, subSample { 0 };
    };

    int numSamples { 1024 };
    int inputSamplesPerBlock { 256 };
    std::array<ChannelInfo, (size_t) numChannels> channels {
        make_array_lambda<ChannelInfo, (size_t) numChannels> ([this] (size_t)
                                                              { return ChannelInfo { *this, numSamples }; })
    };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WaveformView)
};
} // namespace chowdsp

#include "chowdsp_WaveformView.cpp"
