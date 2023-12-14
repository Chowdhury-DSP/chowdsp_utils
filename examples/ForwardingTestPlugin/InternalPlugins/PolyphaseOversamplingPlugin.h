#pragma once

#include <chowdsp_dsp_utils/chowdsp_dsp_utils.h>
#include <chowdsp_filters/chowdsp_filters.h>
#include <chowdsp_plugin_base/chowdsp_plugin_base.h>

struct PolyphaseOversampleParams : chowdsp::ParamHolder
{
    PolyphaseOversampleParams()
    {
        add (gain);
    }

    chowdsp::GainDBParameter::Ptr gain {
        PID { "gain", 100 },
        "Gain",
        juce::NormalisableRange { 0.0f, 30.0f },
        0.0f,
    };
};

class PolyphaseOversamplingPlugin : public chowdsp::PluginBase<chowdsp::PluginStateImpl<PolyphaseOversampleParams>>
{
public:
    PolyphaseOversamplingPlugin() = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    void processAudioBlock (juce::AudioBuffer<float>& buffer) override;

    bool hasEditor() const override { return false; }
    juce::AudioProcessorEditor* createEditor() override { return nullptr; }

private:
    chowdsp::FIRPolyphaseInterpolator<float> upsampler;
    chowdsp::FIRPolyphaseDecimator<float> downsampler;

    chowdsp::Buffer<float> os_buffer {};
    chowdsp::Gain<float> gain {};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PolyphaseOversamplingPlugin)
};
