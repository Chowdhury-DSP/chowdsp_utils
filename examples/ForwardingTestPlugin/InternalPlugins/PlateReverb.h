#pragma once

#include <chowdsp_reverb/chowdsp_reverb.h>
#include <chowdsp_plugin_base/chowdsp_plugin_base.h>

class PlateReverb : public chowdsp::PluginBase<PlateReverb>
{
public:
    PlateReverb();

    static void addParameters (Parameters& params);

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    void processAudioBlock (juce::AudioBuffer<float>& buffer) override;

    bool hasEditor() const override { return false; }
    juce::AudioProcessorEditor* createEditor() override { return nullptr; }

private:
    chowdsp::FloatParameter* inputDiffusionParam = nullptr;
    chowdsp::FloatParameter* decayParam = nullptr;
    chowdsp::FloatParameter* decayDiffusionParam = nullptr;
    chowdsp::FloatParameter* dampingParam = nullptr;
    chowdsp::FloatParameter* mixParam = nullptr;

    chowdsp::Reverb::Dattorro::InputNetwork<> diffusion[2];
    chowdsp::Reverb::Dattorro::TankNetwork<> tank;
    juce::dsp::DryWetMixer<float> mixer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PlateReverb)
};
