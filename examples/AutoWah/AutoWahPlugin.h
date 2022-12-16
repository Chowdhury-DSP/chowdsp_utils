#pragma once

#include <chowdsp_plugin_base/chowdsp_plugin_base.h>
#include <chowdsp_dsp_utils/chowdsp_dsp_utils.h>

class AutoWahPlugin : public chowdsp::PluginBase<AutoWahPlugin>
{
public:
    AutoWahPlugin();

    static void addParameters (Parameters& params);

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    void processAudioBlock (juce::AudioBuffer<float>& buffer) override;

    juce::AudioProcessorEditor* createEditor() override;

private:
    chowdsp::FloatParameter* freqHzParam = nullptr;
    chowdsp::FloatParameter* qValParam = nullptr;
    chowdsp::FloatParameter* gainDBParam = nullptr;
    chowdsp::FloatParameter* attackMsParam = nullptr;
    chowdsp::FloatParameter* releaseMsParam = nullptr;
    chowdsp::FloatParameter* freqModParam = nullptr;

    chowdsp::LevelDetector<float> levelDetector;
    chowdsp::Buffer<float> levelBuffer;

    chowdsp::ModFilterWrapper<chowdsp::PeakingFilter<float>> wahFilter;
    float fs = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AutoWahPlugin)
};
