#pragma once

#include <chowdsp_plugin_utils/chowdsp_plugin_utils.h>
#include <chowdsp_dsp/chowdsp_dsp.h>

#include "ModeParams.h"

class ModalReverbPlugin : public chowdsp::PluginBase<ModalReverbPlugin>
{
public:
    ModalReverbPlugin();

    static void addParameters (Parameters& params);

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    void processAudioBlock (juce::AudioBuffer<float>& buffer) override;

    juce::AudioProcessorEditor* createEditor() override;

private:
    std::atomic<float>* pitchParam = nullptr;
    std::atomic<float>* decayParam = nullptr;
    std::atomic<float>* mixParam = nullptr;

    chowdsp::ModalFilterBank<ModeParams::numModes> modalFilterBank;
    juce::dsp::DryWetMixer<float> mixer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ModalReverbPlugin)
};
