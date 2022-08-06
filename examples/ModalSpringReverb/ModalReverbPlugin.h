#pragma once

#include <chowdsp_plugin_base/chowdsp_plugin_base.h>
#include <chowdsp_dsp_utils/chowdsp_dsp_utils.h>
#include <chowdsp_sources/chowdsp_sources.h>

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
    chowdsp::FloatParameter* pitchParam = nullptr;
    chowdsp::FloatParameter* decayParam = nullptr;
    chowdsp::FloatParameter* mixParam = nullptr;
    chowdsp::FloatParameter* modModesParam = nullptr;
    chowdsp::FloatParameter* modFreqParam = nullptr;
    chowdsp::FloatParameter* modDepthParam = nullptr;

    chowdsp::ModalFilterBank<ModeParams::numModes> modalFilterBank;

    chowdsp::SineWave<float> modSine;
    juce::AudioBuffer<float> modBuffer;

    juce::dsp::DryWetMixer<float> mixer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ModalReverbPlugin)
};
