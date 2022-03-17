#pragma once

#include <chowdsp_plugin_utils/chowdsp_plugin_utils.h>

#include "PrototypeEQ.h"

class LinearPhaseEQPlugin : public chowdsp::PluginBase<LinearPhaseEQPlugin>
{
public:
    LinearPhaseEQPlugin();

    static void addParameters (Parameters& params);

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    void processAudioBlock (juce::AudioBuffer<float>& buffer) override;

    juce::AudioProcessorEditor* createEditor() override;

private:
    void setEQParams (bool force = false);

    std::atomic<float>* lowCutFreqHz = nullptr;
    std::atomic<float>* lowCutQ = nullptr;
    std::atomic<float>* peakingFilterFreqHz = nullptr;
    std::atomic<float>* peakingFilterQ = nullptr;
    std::atomic<float>* peakingFilterGainDB = nullptr;
    std::atomic<float>* highCutFreqHz = nullptr;
    std::atomic<float>* highCutQ = nullptr;
    std::atomic<float>* linPhaseModeOn = nullptr;

    PrototypeEQ protoEQ;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LinearPhaseEQPlugin)
};
