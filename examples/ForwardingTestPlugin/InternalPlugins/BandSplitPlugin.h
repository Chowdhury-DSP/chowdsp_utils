#pragma once

#include <chowdsp_filters/chowdsp_filters.h>
#include <chowdsp_plugin_base/chowdsp_plugin_base.h>

struct BandSplitParams : chowdsp::ParamHolder
{
    BandSplitParams()
    {
        add (freqLowParam,
             freqHighParam,
             orderParam,
             modeParam);
    }

    chowdsp::FreqHzParameter::Ptr freqLowParam {
        juce::ParameterID { "freq_low", 100 },
        "Low Crossover Frequency",
        chowdsp::ParamUtils::createNormalisableRange (20.0f, 20000.0f, 2000.0f),
        200.0f
    };

    chowdsp::FreqHzParameter::Ptr freqHighParam {
        juce::ParameterID { "freq_high", 100 },
        "High Crossover Frequency",
        chowdsp::ParamUtils::createNormalisableRange (20.0f, 20000.0f, 2000.0f),
        2000.0f
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
        juce::StringArray { "Through", "Solo Low", "Solo Mid", "Solo High" },
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
    chowdsp::ThreeWayCrossoverFilter<float, 1> filter1;
    chowdsp::ThreeWayCrossoverFilter<float, 2> filter2;
    chowdsp::ThreeWayCrossoverFilter<float, 4> filter4;
    chowdsp::ThreeWayCrossoverFilter<float, 8> filter8;
    chowdsp::ThreeWayCrossoverFilter<float, 12> filter12;

    chowdsp::Buffer<float> lowBuffer;
    chowdsp::Buffer<float> midBuffer;
    chowdsp::Buffer<float> highBuffer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BandSplitPlugin)
};
