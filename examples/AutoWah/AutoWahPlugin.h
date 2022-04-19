#pragma once

#include <chowdsp_plugin_utils/chowdsp_plugin_utils.h>
#include <chowdsp_dsp/chowdsp_dsp.h>

#include "chowdsp_ModFilterWrapper.h"

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
    std::atomic<float>* freqHzParam = nullptr;
    std::atomic<float>* qValParam = nullptr;
    std::atomic<float>* gainDBParam = nullptr;
    std::atomic<float>* attackMsParam = nullptr;
    std::atomic<float>* releaseMsParam = nullptr;
    std::atomic<float>* freqModParam = nullptr;

    chowdsp::LevelDetector<float> levelDetector;
    juce::AudioBuffer<float> levelBuffer;

    chowdsp::ModFilterWrapper<chowdsp::PeakingFilter<float>> wahFilter;
    float fs = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AutoWahPlugin)
};
