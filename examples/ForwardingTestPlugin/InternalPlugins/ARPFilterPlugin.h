#pragma once

#include <chowdsp_filters/chowdsp_filters.h>
#include <chowdsp_plugin_base/chowdsp_plugin_base.h>

struct ARPFilterParams : chowdsp::ParamHolder
{
    ARPFilterParams()
    {
        add (freqParam,
             qParam,
             limitModeParam,
             notchOffset,
             modeParam);
    }

    chowdsp::FreqHzParameter::Ptr freqParam {
        juce::ParameterID { "freq", 100 },
        "Frequency",
        chowdsp::ParamUtils::createNormalisableRange (20.0f, 20000.0f, 2000.0f),
        1000.0f
    };

    chowdsp::FloatParameter::Ptr qParam {
        juce::ParameterID { "q_val", 100 },
        "Q",
        chowdsp::ParamUtils::createNormalisableRange (0.5f, 2.5f, chowdsp::CoefficientCalculators::butterworthQ<float>),
        chowdsp::CoefficientCalculators::butterworthQ<float>,
        &chowdsp::ParamUtils::floatValToString,
        &chowdsp::ParamUtils::stringToFloatVal
    };

    chowdsp::BoolParameter::Ptr limitModeParam {
        juce::ParameterID { "limit_mode", 100 },
        "Limit Mode",
        false
    };

    chowdsp::PercentParameter::Ptr notchOffset {
        juce::ParameterID { "notch_offset", 100 },
        "Notch Offset",
        0.0f
    };

    chowdsp::ChoiceParameter::Ptr modeParam {
        juce::ParameterID { "mode", 100 },
        "Mode",
        juce::StringArray ("LPF2", "BPF2", "HPF2", "Notch"),
        0
    };
};

class ARPFilterPlugin : public chowdsp::PluginBase<chowdsp::PluginStateImpl<ARPFilterParams>>
{
public:
    ARPFilterPlugin();

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    void processAudioBlock (juce::AudioBuffer<float>& buffer) override;

    bool hasEditor() const override { return false; }
    juce::AudioProcessorEditor* createEditor() override { return nullptr; }

private:
    chowdsp::SmoothedBufferValue<float> notchParam;

    chowdsp::ARPFilter<float> filter;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ARPFilterPlugin)
};
