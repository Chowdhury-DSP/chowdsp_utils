#pragma once

#include <chowdsp_plugin_base/chowdsp_plugin_base.h>
#include <chowdsp_reverb/chowdsp_reverb.h>

class SimpleReverbPlugin : public chowdsp::PluginBase<SimpleReverbPlugin>
{
public:
    SimpleReverbPlugin();

    static void addParameters (Parameters& params);

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    void processAudioBlock (juce::AudioBuffer<float>& buffer) override;

    juce::AudioProcessorEditor* createEditor() override;

private:
    std::atomic<float>* diffusionTimeMsParam = nullptr;
    std::atomic<float>* fdnDelayMsParam = nullptr;
    std::atomic<float>* fdnT60LowMsParam = nullptr;
    std::atomic<float>* fdnT60HighMsParam = nullptr;
    std::atomic<float>* modAmountParam = nullptr;
    std::atomic<float>* dryWetParam = nullptr;

    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Multiplicative> diffusionTimeSmoother;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Multiplicative> fdnTimeSmoother;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Multiplicative> fdnT60LowSmoother;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Multiplicative> fdnT60HighSmoother;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> modAmountSmoother;

    using DiffuserType = chowdsp::Reverb::Diffuser<float, 8>;
    chowdsp::Reverb::DiffuserChain<5, DiffuserType> diffuserChain;

    using FDNConfig = chowdsp::Reverb::DefaultFDNConfig<float, 8>;
    chowdsp::Reverb::FDN<FDNConfig, chowdsp::DelayLineInterpolationTypes::Lagrange3rd> fdn;

    chowdsp::SineWave<float> lfos[2];
    float lfoVals[2] {};

    juce::dsp::DryWetMixer<float> mixer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SimpleReverbPlugin)
};
