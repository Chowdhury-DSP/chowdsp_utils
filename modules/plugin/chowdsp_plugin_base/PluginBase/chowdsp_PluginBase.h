#pragma once

#include <chowdsp_parameters/chowdsp_parameters.h>
#include "chowdsp_ProgramAdapter.h"

#if JUCE_MODULE_AVAILABLE_chowdsp_presets
#include <chowdsp_presets/chowdsp_presets.h>
#endif

#if JUCE_MODULE_AVAILABLE_chowdsp_presets_v2
#include <chowdsp_presets_v2/chowdsp_presets_v2.h>
#endif

namespace chowdsp
{
/**
 * Base class for plugin processors.
 *
 * Derived classes must override `prepareToPlay` and `releaseResources`
 * (from `juce::AudioProcessor`), as well as `processAudioBlock`, and
 * `addParameters`.
*/
#if JUCE_MODULE_AVAILABLE_chowdsp_plugin_state
template <class PluginStateType>
#else
template <class Processor>
#endif
class PluginBase : public juce::AudioProcessor
#if JUCE_MODULE_AVAILABLE_chowdsp_clap_extensions
    ,
                   public CLAPExtensions::CLAPInfoExtensions,
                   public clap_juce_extensions::clap_juce_audio_processor_capabilities
#endif
{
public:
    explicit PluginBase (juce::UndoManager* um = nullptr, const juce::AudioProcessor::BusesProperties& layout = getDefaultBusLayout());
    ~PluginBase() override = default;

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

#if JUCE_MODULE_AVAILABLE_chowdsp_presets_v2
    virtual presets::PresetManager& getPresetManager()
    {
        return *presetManager;
    }
#elif JUCE_MODULE_AVAILABLE_chowdsp_presets
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
#if JUCE_MODULE_AVAILABLE_foleys_gui_magic
    juce::AudioProcessorEditor* createEditor() override
    {
        return new foleys::MagicPluginEditor (magicState);
    }
#endif

    void getStateInformation (juce::MemoryBlock& data) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

#if JUCE_MODULE_AVAILABLE_chowdsp_plugin_state
    auto& getState()
    {
        return state;
    }

    const auto& getState() const
    {
        return state;
    }
#endif

    virtual juce::String getWrapperTypeString() const;
    bool supportsParameterModulation() const;

#if HAS_CLAP_JUCE_EXTENSIONS && JUCE_MODULE_AVAILABLE_chowdsp_presets_v2
    bool supportsPresetLoad() const noexcept override
    {
        return presetManager != nullptr;
    }
    bool presetLoadFromLocation (uint32_t location_kind, const char* location, const char* load_key) noexcept override;
#endif

protected:
#if JUCE_MODULE_AVAILABLE_chowdsp_plugin_state
    PluginStateType state;
#else
    using Parameters = chowdsp::Parameters;
    juce::AudioProcessorValueTreeState vts;

#if JUCE_MODULE_AVAILABLE_foleys_gui_magic
    foleys::MagicProcessorState magicState { *this, vts };
#endif
#endif

#if JUCE_MODULE_AVAILABLE_chowdsp_presets_v2
    std::unique_ptr<presets::PresetManager> presetManager {};
    std::unique_ptr<ProgramAdapter::BaseProgramAdapter> programAdaptor = std::make_unique<ProgramAdapter::BaseProgramAdapter>();
#elif JUCE_MODULE_AVAILABLE_chowdsp_presets
    std::unique_ptr<PresetManager> presetManager;
    std::unique_ptr<ProgramAdapter::BaseProgramAdapter> programAdaptor = std::make_unique<ProgramAdapter::PresetsProgramAdapter> (presetManager);
#else
    std::unique_ptr<ProgramAdapter::BaseProgramAdapter> programAdaptor = std::make_unique<ProgramAdapter::BaseProgramAdapter>();
#endif

private:
    static juce::AudioProcessor::BusesProperties getDefaultBusLayout();

#if ! JUCE_MODULE_AVAILABLE_chowdsp_plugin_state
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    CHOWDSP_CHECK_HAS_STATIC_METHOD (HasAddParameters, addParameters)
#endif

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginBase)
};

template <class P>
juce::AudioProcessor::BusesProperties PluginBase<P>::getDefaultBusLayout()
{
    return BusesProperties()
        .withInput ("Input", juce::AudioChannelSet::stereo(), true)
        .withOutput ("Output", juce::AudioChannelSet::stereo(), true);
}

#if JUCE_MODULE_AVAILABLE_chowdsp_plugin_state
template <class State>
PluginBase<State>::PluginBase (juce::UndoManager* um, const juce::AudioProcessor::BusesProperties& layout)
    : AudioProcessor (layout),
      state (*this, um)
{
}
#else
template <class Processor>
PluginBase<Processor>::PluginBase (juce::UndoManager* um, const juce::AudioProcessor::BusesProperties& layout) : AudioProcessor (layout),
                                                                                                                 vts (*this, um, juce::Identifier ("Parameters"), createParameterLayout())
{
}

template <class Processor>
juce::AudioProcessorValueTreeState::ParameterLayout PluginBase<Processor>::createParameterLayout()
{
    Parameters params;

    static_assert (HasAddParameters<Processor>, "Processor class MUST contain a static addParameters function!");
    Processor::addParameters (params);

    return { params.begin(), params.end() };
}
#endif

template <class P>
int PluginBase<P>::getNumPrograms()
{
    return programAdaptor->getNumPrograms();
}

template <class P>
int PluginBase<P>::getCurrentProgram()
{
    return programAdaptor->getCurrentProgram();
}

template <class P>
void PluginBase<P>::setCurrentProgram (int index)
{
    programAdaptor->setCurrentProgram (index);
}

template <class P>
const juce::String PluginBase<P>::getProgramName (int index) // NOLINT(readability-const-return-type): Needs to return a const String for override compatibility
{
    return programAdaptor->getProgramName (index);
}

template <class P>
void PluginBase<P>::changeProgramName (int index, const juce::String& newName)
{
    programAdaptor->changeProgramName (index, newName);
}

template <class P>
bool PluginBase<P>::isBusesLayoutSupported (const juce::AudioProcessor::BusesLayout& layouts) const
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

template <class P>
void PluginBase<P>::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    setRateAndBufferSizeDetails (sampleRate, samplesPerBlock);
#if JUCE_MODULE_AVAILABLE_foleys_gui_magic
    magicState.prepareToPlay (sampleRate, samplesPerBlock);
#endif
}

template <class P>
void PluginBase<P>::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

#if JUCE_MODULE_AVAILABLE_chowdsp_plugin_state
    state.getParameterListeners().callAudioThreadBroadcasters();
#endif

    processAudioBlock (buffer);
}

#if JUCE_MODULE_AVAILABLE_chowdsp_plugin_state
template <class State>
void PluginBase<State>::getStateInformation (juce::MemoryBlock& data)
{
    state.serialize (data);
}

template <class State>
void PluginBase<State>::setStateInformation (const void* data, int sizeInBytes)
{
    state.deserialize (juce::MemoryBlock { data, (size_t) sizeInBytes });
}
#else
template <class Processor>
void PluginBase<Processor>::getStateInformation (juce::MemoryBlock& data)
{
#if JUCE_MODULE_AVAILABLE_foleys_gui_magic
    magicState.getStateInformation (data);
#else
    auto state = vts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, data);
#endif
}

template <class Processor>
void PluginBase<Processor>::setStateInformation (const void* data, int sizeInBytes)
{
#if JUCE_MODULE_AVAILABLE_foleys_gui_magic
    magicState.setStateInformation (data, sizeInBytes, getActiveEditor());
#else
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));

    if (xmlState != nullptr)
        if (xmlState->hasTagName (vts.state.getType()))
            vts.replaceState (juce::ValueTree::fromXml (*xmlState));
#endif
}
#endif

template <class P>
juce::String PluginBase<P>::getWrapperTypeString() const
{
#if JUCE_MODULE_AVAILABLE_chowdsp_clap_extensions
    return CLAPExtensions::CLAPInfoExtensions::getPluginTypeString (wrapperType);
#else
    return AudioProcessor::getWrapperTypeDescription (wrapperType);
#endif
}

template <class P>
bool PluginBase<P>::supportsParameterModulation() const
{
#if JUCE_MODULE_AVAILABLE_chowdsp_clap_extensions
    return CLAPExtensions::CLAPInfoExtensions::is_clap;
#else
    return false;
#endif
}

#if HAS_CLAP_JUCE_EXTENSIONS && JUCE_MODULE_AVAILABLE_chowdsp_presets_v2
template <class P>
bool PluginBase<P>::presetLoadFromLocation (uint32_t location_kind, const char* location, const char* load_key) noexcept
{
    if (presetManager == nullptr)
        return false;
    return presetManager->loadCLAPPreset (location_kind, location, load_key);
}
#endif
} // namespace chowdsp
