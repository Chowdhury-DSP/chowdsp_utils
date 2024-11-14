#pragma once

#include <chowdsp_plugin_base/chowdsp_plugin_base.h>

#include "../SignalGenerator/SignalGeneratorPlugin.h"
#include "../SimpleReverb/SimpleReverbPlugin.h"
#include "InternalPlugins/WernerFilterPlugin.h"
#include "InternalPlugins/ARPFilterPlugin.h"
#include "InternalPlugins/PolygonalOscPlugin.h"
#include "InternalPlugins/BandSplitPlugin.h"
#include "InternalPlugins/PlateReverb.h"
#include "InternalPlugins/PolyphaseOversamplingPlugin.h"

struct PluginParams : chowdsp::ParamHolder
{
    PluginParams()
    {
        add (processorChoice);
    }

    chowdsp::ChoiceParameter::Ptr processorChoice {
        chowdsp::ParameterID { "processor_choice", 100 },
        "Processor Choice",
        juce::StringArray {
            "None",
            "Tone Generator",
            "Reverb",
            "Werner Filter",
            "ARP Filter",
            "Polygonal Oscillator",
            "Band Split",
            "Plate Reverb",
            "Polyphase Oversampling",
        },
        0
    };
};

class ForwardingTestPlugin : public chowdsp::PluginBase<chowdsp::PluginStateImpl<PluginParams>>
{
public:
    ForwardingTestPlugin();
    ~ForwardingTestPlugin() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    void processAudioBlock (juce::AudioBuffer<float>& buffer) override;

    juce::AudioProcessorEditor* createEditor() override;

    juce::AudioProcessor* getProcessorForIndex (int index);

private:
    SignalGeneratorPlugin toneGenerator;
    SimpleReverbPlugin reverb;
    WernerFilterPlugin wernerFilter;
    ARPFilterPlugin arpFilter;
    PolygonalOscPlugin polygonalOsc;
    BandSplitPlugin bandSplit;
    PlateReverb plateReverb;
    PolyphaseOversamplingPlugin polyphaseOversampling;

    struct ParamForwardingProvider
    {
        static chowdsp::ParameterID getForwardingParameterID (int paramIndex)
        {
            return { "forward_param_" + juce::String (paramIndex), 100 };
        }
    };

    static constexpr int numForwardParameters = 10;
    using ForwardingParams = chowdsp::ForwardingParametersManager<ParamForwardingProvider, numForwardParameters>;
    ForwardingParams forwardingParameters;

    chowdsp::ScopedCallback processorChangedCallback;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ForwardingTestPlugin)
};
