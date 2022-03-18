#pragma once

#include <chowdsp_plugin_utils/chowdsp_plugin_utils.h>

#include "PrototypeEQ.h"

/** Example plugin to demonstrate the use of chowdsp::LinearPhaseEQ */
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
    PrototypeEQ::Params makeEQParams() const;
    void setEQParams();

    std::atomic<float>* lowCutFreqHz = nullptr;
    std::atomic<float>* lowCutQ = nullptr;
    std::atomic<float>* peakingFilterFreqHz = nullptr;
    std::atomic<float>* peakingFilterQ = nullptr;
    std::atomic<float>* peakingFilterGainDB = nullptr;
    std::atomic<float>* highCutFreqHz = nullptr;
    std::atomic<float>* highCutQ = nullptr;
    std::atomic<float>* linPhaseModeOn = nullptr;

    // In general, you only need to create a linear phase EQ, but in this case
    // we want to be able to A/B test, so we'll create both the prototype and
    // the linear phase EQs
    PrototypeEQ protoEQ;
    chowdsp::LinearPhaseEQ<PrototypeEQ> linPhaseEQ;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LinearPhaseEQPlugin)
};
