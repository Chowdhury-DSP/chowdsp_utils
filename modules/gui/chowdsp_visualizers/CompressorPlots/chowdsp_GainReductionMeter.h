#pragma once

#if JUCE_MODULE_AVAILABLE_chowdsp_plugin_utils && JUCE_MODULE_AVAILABLE_juce_dsp
#include <chowdsp_plugin_utils/chowdsp_plugin_utils.h>

namespace chowdsp::compressor
{
/** Generic UI component for implementing a gain reduction meter */
class GainReductionMeter : public juce::Component,
                           private juce::Timer
{
public:
    struct BackgroundTask : TimeSliceAudioUIBackgroundTask
    {
        BackgroundTask() : TimeSliceAudioUIBackgroundTask ("Gain Reduction Meter Background Task") {}

        void prepareTask (double sampleRate, int samplesPerBlock, int& requestedBlockSize, int& waitMs) override;
        void runTask (const juce::AudioBuffer<float>& data) override;

        [[nodiscard]] float getGainReductionDB() const;

        void pushBufferData (const BufferView<const float>& buffer, bool isInput);

    private:
        std::atomic<float> gainReductionDB {};
        juce::dsp::BallisticsFilter<float> ballisticsFilter;
        Buffer<float> meterBuffer;
    };

    explicit GainReductionMeter (BackgroundTask& task);
    ~GainReductionMeter() override;

    void paint (juce::Graphics& g) override;

protected:
    void timerCallback() override { repaint(); }

    BackgroundTask& task;
    juce::SmoothedValue<float> gainReductionSmoother;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GainReductionMeter)
};
} // namespace chowdsp::compressor

#endif // JUCE_MODULE_AVAILABLE_chowdsp_plugin_utils && JUCE_MODULE_AVAILABLE_juce_dsp
