#pragma once

#include <chowdsp_filters/chowdsp_filters.h>
#include <chowdsp_plugin_base/chowdsp_plugin_base.h>

struct BandSplitParams : chowdsp::ParamHolder
{
    BandSplitParams()
    {
        add (freqParam,
             orderParam,
             modeParam);
    }

    chowdsp::FreqHzParameter::Ptr freqParam {
        juce::ParameterID { "freq", 100 },
        "Crossover Frequency",
        chowdsp::ParamUtils::createNormalisableRange (20.0f, 20000.0f, 2000.0f),
        1000.0f
    };

    chowdsp::ChoiceParameter::Ptr orderParam {
        juce::ParameterID { "order", 100 },
        "Order",
        juce::StringArray { "1", "2", "4", "8", "12" },
        0
    };

    chowdsp::ChoiceParameter::Ptr modeParam {
        juce::ParameterID { "mode", 100 },
        "Mode",
        juce::StringArray { "Through", "Mute Low", "Mute High" },
        0
    };
};

class BandSplitPlugin : public chowdsp::PluginBase<chowdsp::PluginStateImpl<BandSplitParams>>
{
public:
    BandSplitPlugin();

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    void processAudioBlock (juce::AudioBuffer<float>& buffer) override;

    bool hasEditor() const override { return false; }
    juce::AudioProcessorEditor* createEditor() override { return nullptr; }

private:
    chowdsp::LinkwitzRileyFilter<float, 1> filter1;
    chowdsp::LinkwitzRileyFilter<float, 2> filter2;
    chowdsp::LinkwitzRileyFilter<float, 4> filter4;
    chowdsp::LinkwitzRileyFilter<float, 8> filter8;
    chowdsp::LinkwitzRileyFilter<float, 12> filter12;

    chowdsp::Buffer<float> lowBuffer;
    chowdsp::Buffer<float> highBuffer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BandSplitPlugin)
};
