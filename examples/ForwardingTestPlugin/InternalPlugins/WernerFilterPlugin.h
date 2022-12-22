#pragma once

#include <chowdsp_filters/chowdsp_filters.h>
#include <chowdsp_plugin_base/chowdsp_plugin_base.h>

struct WernerFilterParams : chowdsp::ParamHolder
{
    WernerFilterParams()
    {
        add (freqParam,
             resonanceParam,
             dampingParam,
             morphParam,
             modeParam);
    }

    chowdsp::FreqHzParameter::Ptr freqParam {
        juce::ParameterID { "freq", 100 },
        "Frequency",
        chowdsp::ParamUtils::createNormalisableRange (20.0f, 20000.0f, 2000.0f),
        1000.0f
    };

    chowdsp::PercentParameter::Ptr resonanceParam {
        juce::ParameterID { "resonance", 100 },
        "Resonance",
        0.5f
    };

    chowdsp::PercentParameter::Ptr dampingParam {
        juce::ParameterID { "damping", 100 },
        "Damping",
        0.5f
    };

    chowdsp::PercentParameter::Ptr morphParam {
        juce::ParameterID { "morph", 100 },
        "Morph",
        0.0f
    };

    chowdsp::ChoiceParameter::Ptr modeParam {
        juce::ParameterID { "mode", 100 },
        "Mode",
        juce::StringArray ("LPF2", "BPF2", "HPF2", "Multi-Mode", "LPF4"),
        3
    };
};

class WernerFilterPlugin : public chowdsp::PluginBase<chowdsp::PluginStateImpl<WernerFilterParams>>
{
public:
    WernerFilterPlugin();

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    void processAudioBlock (juce::AudioBuffer<float>& buffer) override;

    bool hasEditor() const override { return false; }
    juce::AudioProcessorEditor* createEditor() override { return nullptr; }

private:
    chowdsp::SmoothedBufferValue<float> morphParam;

    chowdsp::WernerFilter filter;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WernerFilterPlugin)
};
