#pragma once

#include <chowdsp_reverb/chowdsp_reverb.h>
#include <chowdsp_plugin_base/chowdsp_plugin_base.h>

struct PlateParams : chowdsp::ParamHolder
{
    PlateParams()
    {
        add (inputDiffusion,
             decay,
             decayDiffusion,
             damping,
             mix);
    }

    chowdsp::PercentParameter::Ptr inputDiffusion {
        juce::ParameterID { "in_diffusion", 100 },
        "Input Diffusion",
        0.5f
    };

    chowdsp::PercentParameter::Ptr decay {
        juce::ParameterID { "decay", 100 },
        "Decay",
        0.5f
    };

    chowdsp::PercentParameter::Ptr decayDiffusion {
        juce::ParameterID { "decay_diffusion", 100 },
        "Decay Diffusion",
        0.5f
    };

    chowdsp::FreqHzParameter::Ptr damping {
        juce::ParameterID { "damping", 100 },
        "Damping",
        chowdsp::ParamUtils::createNormalisableRange (2000.0f, 20000.0f, 10000.0f),
        10000.0f
    };

    chowdsp::PercentParameter::Ptr mix {
        juce::ParameterID { "mix", 100 },
        "Mix",
        0.5f
    };
};

class PlateReverb : public chowdsp::PluginBase<chowdsp::PluginStateImpl<PlateParams>>
{
public:
    PlateReverb();

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    void processAudioBlock (juce::AudioBuffer<float>& buffer) override;

    bool hasEditor() const override { return false; }
    juce::AudioProcessorEditor* createEditor() override { return nullptr; }

private:
    chowdsp::Reverb::Dattorro::InputNetwork<> diffusion[2];
    chowdsp::Reverb::Dattorro::TankNetwork<> tank;
    juce::dsp::DryWetMixer<float> mixer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PlateReverb)
};
