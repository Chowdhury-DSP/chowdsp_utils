#pragma once

#include <chowdsp_filters/chowdsp_filters.h>
#include <chowdsp_plugin_base/chowdsp_plugin_base.h>

struct BandSplitParams : chowdsp::ParamHolder
{
    BandSplitParams()
    {
        add (freqLowParam,
             freqMidParam,
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

    chowdsp::FreqHzParameter::Ptr freqMidParam {
        juce::ParameterID { "freq_mid", 100 },
        "Mid Crossover Frequency",
        chowdsp::ParamUtils::createNormalisableRange (20.0f, 20000.0f, 2000.0f),
        1000.0f
    };

    chowdsp::FreqHzParameter::Ptr freqHighParam {
        juce::ParameterID { "freq_high", 100 },
        "High Crossover Frequency",
        chowdsp::ParamUtils::createNormalisableRange (20.0f, 20000.0f, 2000.0f),
        8000.0f
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
        juce::StringArray { "Through", "Solo Low", "Solo Mid-Low", "Solo Mid-High", "Solo High" },
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
    static constexpr auto numBands = 4;
    chowdsp::CrossoverFilter<float, 1, numBands> filter1;
    chowdsp::CrossoverFilter<float, 2, numBands> filter2;
    chowdsp::CrossoverFilter<float, 4, numBands> filter4;
    chowdsp::CrossoverFilter<float, 8, numBands> filter8;
    chowdsp::CrossoverFilter<float, 12, numBands> filter12;

    std::array<chowdsp::Buffer<float>, numBands> outBuffers;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BandSplitPlugin)
};
