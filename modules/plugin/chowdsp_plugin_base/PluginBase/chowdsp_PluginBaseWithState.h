#pragma once

#include <chowdsp_parameters/chowdsp_parameters.h>
#include "chowdsp_ProgramAdapter.h"

namespace chowdsp
{
/**
 * Base class for plugin processors.
 * 
 * Derived classes must override `prepareToPlay` and `releaseResources`
 * (from `juce::AudioProcessor`), as well as `processAudioBlock`, and
 * `addParameters`.
*/
template <class PluginStateType>
class PluginBaseWithState : public juce::AudioProcessor
#if JUCE_MODULE_AVAILABLE_chowdsp_clap_extensions
    ,
                            public CLAPExtensions::CLAPInfoExtensions,
                            public clap_juce_extensions::clap_juce_audio_processor_capabilities
#endif
{
public:
    explicit PluginBaseWithState (juce::UndoManager* um = nullptr, const juce::AudioProcessor::BusesProperties& layout = getDefaultBusLayout());
    ~PluginBaseWithState() override = default;

#if defined JucePlugin_Name
    const juce::String getName() const override // NOLINT(readability-const-return-type): Needs to return a const String for override compatibility
    {
        return JucePlugin_Name;
    }
#else
    const juce::String getName() const override // NOLINT(readability-const-return-type): Needs to return a const String for override compatibility
    {
        return juce::String();
    }
#endif

    bool acceptsMidi() const override
    {
        return false;
    }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }

    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int) override;
    const juce::String getProgramName (int) override;
    void changeProgramName (int, const juce::String&) override;

#if JUCE_MODULE_AVAILABLE_chowdsp_presets
    virtual PresetManager& getPresetManager()
    {
        return *presetManager;
    }

    void setUsePresetManagerForPluginInterface (bool shouldUse)
    {
        programAdaptor = shouldUse
                             ? std::make_unique<ProgramAdapter::PresetsProgramAdapter> (presetManager)
                             : std::make_unique<ProgramAdapter::BaseProgramAdapter>();
    }
#endif

    bool isBusesLayoutSupported (const juce::AudioProcessor::BusesLayout& layouts) const override;
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    void processBlock (juce::AudioBuffer<double>&, juce::MidiBuffer&) override {}
    virtual void processAudioBlock (juce::AudioBuffer<float>&) = 0;

    bool hasEditor() const override
    {
        return true;
    }

    void getStateInformation (juce::MemoryBlock& data) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    auto& getState() { return state; }
    const auto& getState() const { return state; }

    virtual juce::String getWrapperTypeString() const;
    bool supportsParameterModulation() const;

protected:
    PluginStateType state;

#if JUCE_MODULE_AVAILABLE_chowdsp_presets
    std::unique_ptr<chowdsp::PresetManager> presetManager;
    std::unique_ptr<ProgramAdapter::BaseProgramAdapter> programAdaptor = std::make_unique<ProgramAdapter::PresetsProgramAdapter> (presetManager);
#else
    std::unique_ptr<ProgramAdapter::BaseProgramAdapter> programAdaptor = std::make_unique<ProgramAdapter::BaseProgramAdapter>();
#endif

private:
    static juce::AudioProcessor::BusesProperties getDefaultBusLayout();

    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginBaseWithState)
};

template <class Processor>
juce::AudioProcessor::BusesProperties PluginBaseWithState<Processor>::getDefaultBusLayout()
{
    return BusesProperties()
        .withInput ("Input", juce::AudioChannelSet::stereo(), true)
        .withOutput ("Output", juce::AudioChannelSet::stereo(), true);
}

template <class Processor>
PluginBaseWithState<Processor>::PluginBaseWithState (juce::UndoManager* um, const juce::AudioProcessor::BusesProperties& layout)
    : AudioProcessor (layout),
      state (*this)
{
    // @TODO: get new state working with UndoManager
    juce::ignoreUnused (um);
}

template <class Processor>
int PluginBaseWithState<Processor>::getNumPrograms()
{
    return programAdaptor->getNumPrograms();
}

template <class Processor>
int PluginBaseWithState<Processor>::getCurrentProgram()
{
    return programAdaptor->getCurrentProgram();
}

template <class Processor>
void PluginBaseWithState<Processor>::setCurrentProgram (int index)
{
    programAdaptor->setCurrentProgram (index);
}

template <class Processor>
const juce::String PluginBaseWithState<Processor>::getProgramName (int index) // NOLINT(readability-const-return-type): Needs to return a const String for override compatibility
{
    return programAdaptor->getProgramName (index);
}

template <class Processor>
void PluginBaseWithState<Processor>::changeProgramName (int index, const juce::String& newName)
{
    programAdaptor->changeProgramName (index, newName);
}

template <class Processor>
bool PluginBaseWithState<Processor>::isBusesLayoutSupported (const juce::AudioProcessor::BusesLayout& layouts) const
{
    // only supports mono and stereo (for now)
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // input and output layout must be the same
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;

    return true;
}

template <class Processor>
void PluginBaseWithState<Processor>::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    setRateAndBufferSizeDetails (sampleRate, samplesPerBlock);
#if JUCE_MODULE_AVAILABLE_foleys_gui_magic
    magicState.prepareToPlay (sampleRate, samplesPerBlock);
#endif
}

template <class Processor>
void PluginBaseWithState<Processor>::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    processAudioBlock (buffer);
}

template <class Processor>
void PluginBaseWithState<Processor>::getStateInformation (juce::MemoryBlock& data)
{
    state.serialize (data);
}

template <class Processor>
void PluginBaseWithState<Processor>::setStateInformation (const void* data, int sizeInBytes)
{
    state.deserialize (juce::MemoryBlock { data, (size_t) sizeInBytes });
}

template <class Processor>
juce::String PluginBaseWithState<Processor>::getWrapperTypeString() const
{
#if JUCE_MODULE_AVAILABLE_chowdsp_clap_extensions
    return CLAPExtensions::CLAPInfoExtensions::getPluginTypeString (wrapperType);
#else
    return AudioProcessor::getWrapperTypeDescription (wrapperType);
#endif
}

template <class Processor>
bool PluginBaseWithState<Processor>::supportsParameterModulation() const
{
#if JUCE_MODULE_AVAILABLE_chowdsp_clap_extensions
    return CLAPExtensions::CLAPInfoExtensions::is_clap;
#else
    return false;
#endif
}
} // namespace chowdsp
