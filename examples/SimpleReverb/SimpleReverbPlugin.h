#pragma once

#include <chowdsp_plugin_base/chowdsp_plugin_base.h>
#include <chowdsp_reverb/chowdsp_reverb.h>
#include <chowdsp_sources/chowdsp_sources.h>

struct ReverbParams : chowdsp::ParamHolder
{
    ReverbParams()
    {
        add (diffusionTime,
             fdnDelay,
             fdnT60Low,
             fdnT60High,
             modAmount,
             dryWet);
    }

    chowdsp::TimeMsParameter::Ptr diffusionTime {
        juce::ParameterID { "diffusion", 100 },
        "Diffusion Time",
        chowdsp::ParamUtils::createNormalisableRange (10.0f, 1000.0f, 100.0f),
        100.0f
    };
    chowdsp::TimeMsParameter::Ptr fdnDelay {
        juce::ParameterID { "delay", 100 },
        "FDN Delay Time",
        chowdsp::ParamUtils::createNormalisableRange (50.0f, 500.0f, 150.0f),
        100.0f
    };
    chowdsp::TimeMsParameter::Ptr fdnT60Low {
        juce::ParameterID { "t60_low", 100 },
        "FDN T60 Low",
        chowdsp::ParamUtils::createNormalisableRange (100.0f, 5000.0f, 1000.0f),
        500.0f
    };
    chowdsp::TimeMsParameter::Ptr fdnT60High {
        juce::ParameterID { "t60_high", 100 },
        "FDN T60 High",
        chowdsp::ParamUtils::createNormalisableRange (100.0f, 5000.0f, 1000.0f),
        500.0f
    };
    chowdsp::PercentParameter::Ptr modAmount {
        juce::ParameterID { "mod_amount", 100 },
        "Modulation",
        0.0f
    };
    chowdsp::PercentParameter::Ptr dryWet {
        juce::ParameterID { "dry_wet", 100 },
        "Dry/Wet",
        0.25f
    };
};

class SimpleReverbPlugin : public chowdsp::PluginBase<chowdsp::PluginStateImpl<ReverbParams>>
{
public:
    SimpleReverbPlugin();

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    void processAudioBlock (juce::AudioBuffer<float>& buffer) override;

    juce::AudioProcessorEditor* createEditor() override;

private:
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Multiplicative> diffusionTimeSmoother;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Multiplicative> fdnTimeSmoother;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Multiplicative> fdnT60LowSmoother;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Multiplicative> fdnT60HighSmoother;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> modAmountSmoother;

    chowdsp::Reverb::ConvolutionDiffuser diffuser { 1.0 };

    using FDNConfig = chowdsp::Reverb::DefaultFDNConfig<float, 8>;
    chowdsp::Reverb::FDN<FDNConfig, chowdsp::DelayLineInterpolationTypes::None> fdn;

    chowdsp::SineWave<float> lfos[2];
    float lfoVals[2] {};

    juce::dsp::DryWetMixer<float> mixer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SimpleReverbPlugin)
};
