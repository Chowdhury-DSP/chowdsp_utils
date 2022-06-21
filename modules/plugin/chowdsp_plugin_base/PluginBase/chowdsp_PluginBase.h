#pragma once

#include <chowdsp_parameters/chowdsp_parameters.h>
#include "chowdsp_ProgramAdapter.h"

#if HAS_CLAP_JUCE_EXTENSIONS
JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wunused-parameter")
#include "clap-juce-extensions/clap-juce-extensions.h"
JUCE_END_IGNORE_WARNINGS_GCC_LIKE
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
template <class Processor>
class PluginBase : public juce::AudioProcessor
#if HAS_CLAP_JUCE_EXTENSIONS
    ,
                   protected clap_juce_extensions::clap_properties,
                   public clap_juce_extensions::clap_extensions
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

#if HAS_CLAP_JUCE_EXTENSIONS
    bool supportsDirectProcess() override
    {
        return true;
    }
    clap_process_status clap_direct_process (const clap_process* process) noexcept override;
#endif

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

#if HAS_CLAP_JUCE_EXTENSIONS
    void process_clap_event (const clap_event_header_t* event, juce::MidiBuffer& midiBuffer, int sampleOffset);
#endif

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

#if HAS_CLAP_JUCE_EXTENSIONS
template <class Processor>
clap_process_status PluginBase<Processor>::clap_direct_process (const clap_process* process) noexcept
{
    auto events = process->in_events;
    auto numEvents = (int) events->size (events);
    int currentEvent = 0;
    int nextEventTime = -1;

    if (numEvents > 0)
    {
        auto event = events->get (events, 0);
        nextEventTime = (int) event->time;
    }

    // We process in place so
    static constexpr uint32_t maxBuses = 128;
    std::array<float*, maxBuses> busses {};
    busses.fill (nullptr);
    juce::MidiBuffer midiBuffer; // no midi in this plugin...

    static constexpr int smallBlockSize = 64;
    const auto numSamples = (int) process->frames_count;
    for (int n = 0; n < numSamples; n += smallBlockSize)
    {
        const auto numSamplesToProcess = juce::jmin (smallBlockSize, numSamples - n);

        while (nextEventTime >= 0 && nextEventTime < n + numSamplesToProcess && currentEvent < numEvents)
        {
            auto event = events->get (events, (uint32_t) currentEvent);
            process_clap_event (event, midiBuffer, n);

            currentEvent++;
            nextEventTime = (currentEvent < numEvents) ? (int) events->get (events, (uint32_t) currentEvent)->time : -1;
        }

        uint32_t outputChannels = 0;
        for (uint32_t idx = 0; idx < process->audio_outputs_count && outputChannels < maxBuses; ++idx)
        {
            for (uint32_t ch = 0; ch < process->audio_outputs[idx].channel_count; ++ch)
            {
                busses[outputChannels] = process->audio_outputs[idx].data32[ch] + n;
                outputChannels++;
            }
        }

        uint32_t inputChannels = 0;
        for (uint32_t idx = 0; idx < process->audio_inputs_count && inputChannels < maxBuses; ++idx)
        {
            for (uint32_t ch = 0; ch < process->audio_inputs[idx].channel_count; ++ch)
            {
                auto* ic = process->audio_inputs[idx].data32[ch] + n;
                if (inputChannels < outputChannels)
                {
                    if (ic == busses[inputChannels])
                    {
                        // The buffers overlap - no need to do anything
                    }
                    else
                    {
                        juce::FloatVectorOperations::copy (busses[inputChannels], ic, numSamplesToProcess);
                    }
                }
                else
                {
                    busses[inputChannels] = ic;
                }
                inputChannels++;
            }
        }

        auto totalChans = juce::jmax (inputChannels, outputChannels);
        juce::AudioBuffer<float> buffer (busses.data(), (int) totalChans, numSamplesToProcess);

        processBlock (buffer, midiBuffer);

        midiBuffer.clear();
    }

    // process any leftover events
    for (; currentEvent < numEvents; ++currentEvent)
    {
        auto event = events->get (events, (uint32_t) currentEvent);
        process_clap_event (event, midiBuffer, 0);
    }

    return CLAP_PROCESS_CONTINUE;
}

template <class Processor>
void PluginBase<Processor>::process_clap_event (const clap_event_header_t* event, juce::MidiBuffer& midiBuffer, int sampleOffset)
{
    if (event->space_id != CLAP_CORE_EVENT_SPACE_ID)
        return;

    switch (event->type)
    {
        case CLAP_EVENT_NOTE_ON:
        {
            auto noteEvent = reinterpret_cast<const clap_event_note*> (event);

            midiBuffer.addEvent (juce::MidiMessage::noteOn (noteEvent->channel + 1,
                                                            noteEvent->key,
                                                            (float)
                                                                noteEvent->velocity),
                                 (int) noteEvent->header.time - sampleOffset);
        }
        break;
        case CLAP_EVENT_NOTE_OFF:
        {
            auto noteEvent = reinterpret_cast<const clap_event_note*> (event);

            midiBuffer.addEvent (juce::MidiMessage::noteOff (noteEvent->channel + 1,
                                                             noteEvent->key,
                                                             (float)
                                                                 noteEvent->velocity),
                                 (int) noteEvent->header.time - sampleOffset);
        }
        break;
        case CLAP_EVENT_MIDI:
        {
            auto midiEvent = reinterpret_cast<const clap_event_midi*> (event);
            midiBuffer.addEvent (juce::MidiMessage (midiEvent->data[0],
                                                    midiEvent->data[1],
                                                    midiEvent->data[2],
                                                    (double) midiEvent->header.time - sampleOffset),
                                 (int) midiEvent->header.time - sampleOffset);
        }
        break;
        case CLAP_EVENT_PARAM_VALUE:
        {
            auto paramEvent = reinterpret_cast<const clap_event_param_value*> (event);
            auto juceParameter = static_cast<juce::AudioProcessorParameter*> (paramEvent->cookie);

            if (juceParameter->getValue() == (float) paramEvent->value)
                return;

            juceParameter->setValueNotifyingHost ((float) paramEvent->value);
        }
        break;
        case CLAP_EVENT_PARAM_MOD:
        {
            auto paramModEvent = reinterpret_cast<const clap_event_param_mod*> (event);
            auto modulatableParameter = static_cast<chowdsp::FloatParameter*> (paramModEvent->cookie);

            if (paramModEvent->note_id >= 0)
            {
                //                if (modulatableParameter->supportsMonophonicModulation())
                //                    modulatableParameter->applyPolyphonicModulation (paramModEvent->note_id, paramModEvent->key, paramModEvent->channel, paramModEvent->amount);
            }
            else
            {
                if (modulatableParameter->supportsMonophonicModulation())
                    modulatableParameter->applyMonophonicModulation (paramModEvent->amount);
            }
        }
        break;
        default:
            break;
    }
}
#endif

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
#if HAS_CLAP_JUCE_EXTENSIONS
    // Since we are using 'external clap' this is the one JUCE API we can't override
    if (wrapperType == wrapperType_Undefined && is_clap)
        return "CLAP";
#endif

    return AudioProcessor::getWrapperTypeDescription (wrapperType);
}

} // namespace chowdsp
