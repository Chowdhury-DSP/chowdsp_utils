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
template <class Processor>
class PluginBase : public juce::AudioProcessor
#if JUCE_MODULE_AVAILABLE_chowdsp_clap_extensions
    ,
                   public CLAPExtensions::CLAPProcessorExtensions,
                   private CLAPExtensions::CLAPInfoExtensions
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
#if JUCE_MODULE_AVAILABLE_foleys_gui_magic
    juce::AudioProcessorEditor* createEditor() override
    {
        return new foleys::MagicPluginEditor (magicState);
    }
#endif

    void getStateInformation (juce::MemoryBlock& data) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    virtual juce::String getWrapperTypeString() const;

protected:
    using Parameters = chowdsp::Parameters;
    juce::AudioProcessorValueTreeState vts;

#if JUCE_MODULE_AVAILABLE_foleys_gui_magic
    foleys::MagicProcessorState magicState { *this, vts };
#endif

#if JUCE_MODULE_AVAILABLE_chowdsp_presets
    std::unique_ptr<chowdsp::PresetManager> presetManager;
    std::unique_ptr<ProgramAdapter::BaseProgramAdapter> programAdaptor = std::make_unique<ProgramAdapter::PresetsProgramAdapter> (presetManager);
#else
    std::unique_ptr<ProgramAdapter::BaseProgramAdapter> programAdaptor = std::make_unique<ProgramAdapter::BaseProgramAdapter>();
#endif

private:
    static juce::AudioProcessor::BusesProperties getDefaultBusLayout();

    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    CHOWDSP_CHECK_HAS_STATIC_METHOD (HasAddParameters, addParameters)

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginBase)
};

template <class Processor>
juce::AudioProcessor::BusesProperties PluginBase<Processor>::getDefaultBusLayout()
{
    return BusesProperties()
        .withInput ("Input", juce::AudioChannelSet::stereo(), true)
        .withOutput ("Output", juce::AudioChannelSet::stereo(), true);
}

template <class Processor>
PluginBase<Processor>::PluginBase (juce::UndoManager* um, const juce::AudioProcessor::BusesProperties& layout) : AudioProcessor (layout),
                                                                                                                 vts (*this, um, juce::Identifier ("Parameters"), createParameterLayout())
{
#if JUCE_MODULE_AVAILABLE_chowdsp_clap_extensions
    CLAPExtensions::CLAPProcessorExtensions::initialise (*this);
#endif
}

template <class Processor>
juce::AudioProcessorValueTreeState::ParameterLayout PluginBase<Processor>::createParameterLayout()
{
    Parameters params;

    static_assert (HasAddParameters<Processor>, "Processor class MUST contain a static addParameters function!");
    Processor::addParameters (params);

    return { params.begin(), params.end() };
}

template <class Processor>
int PluginBase<Processor>::getNumPrograms()
{
    return programAdaptor->getNumPrograms();
}

template <class Processor>
int PluginBase<Processor>::getCurrentProgram()
{
    return programAdaptor->getCurrentProgram();
}

template <class Processor>
void PluginBase<Processor>::setCurrentProgram (int index)
{
    programAdaptor->setCurrentProgram (index);
}

template <class Processor>
const juce::String PluginBase<Processor>::getProgramName (int index) // NOLINT(readability-const-return-type): Needs to return a const String for override compatibility
{
    return programAdaptor->getProgramName (index);
}

template <class Processor>
void PluginBase<Processor>::changeProgramName (int index, const juce::String& newName)
{
    programAdaptor->changeProgramName (index, newName);
}

template <class Processor>
bool PluginBase<Processor>::isBusesLayoutSupported (const juce::AudioProcessor::BusesLayout& layouts) const
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
void PluginBase<Processor>::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    setRateAndBufferSizeDetails (sampleRate, samplesPerBlock);
#if JUCE_MODULE_AVAILABLE_foleys_gui_magic
    magicState.prepareToPlay (sampleRate, samplesPerBlock);
#endif
}

template <class Processor>
void PluginBase<Processor>::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    processAudioBlock (buffer);
}

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

template <class Processor>
juce::String PluginBase<Processor>::getWrapperTypeString() const
{
#if JUCE_MODULE_AVAILABLE_chowdsp_clap_extensions
    return CLAPExtensions::CLAPInfoExtensions::getPluginTypeString (wrapperType);
#else
    return AudioProcessor::getWrapperTypeDescription (wrapperType);
#endif
}

} // namespace chowdsp
