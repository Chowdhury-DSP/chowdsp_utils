#pragma once

namespace chowdsp
{
/**
 * Base class for synth plugin processors.
 * 
 * Derived classes must override `prepareToPlay` and `releaseResources`
 * (from `juce::AudioProcessor`), as well as `processSynth`, and
 * `addParameters`.
*/
template <class Processor>
class SynthBase : public PluginBase<Processor>
{
public:
    explicit SynthBase (juce::UndoManager* um = nullptr, const juce::AudioProcessor::BusesProperties& layout = getDefaultBusLayout()) : PluginBase<Processor> (um, layout)
    {
    }

    ~SynthBase() override = default;

    [[nodiscard]] bool acceptsMidi() const override { return true; }

    virtual void processSynth (juce::AudioBuffer<float>&, juce::MidiBuffer&) = 0;

    void processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi) override
    {
        juce::ScopedNoDenormals noDenormals;

#if JUCE_MODULE_AVAILABLE_chowdsp_plugin_state
        this->state.getParameterListeners().callAudioThreadBroadcasters();
#endif

#if JUCE_MODULE_AVAILABLE_foleys_gui_magic
        this->magicState.processMidiBuffer (midi, buffer.getNumSamples(), true);
#endif

        processSynth (buffer, midi);
    }

#if JUCE_MODULE_AVAILABLE_chowdsp_clap_extensions
    [[nodiscard]] bool supportsNoteName() const noexcept override
    {
        return true;
    }
    uint32_t noteNameCount() noexcept override { return 0; }
    bool noteNameGet (uint32_t /*index*/, clap_note_name* /*noteName*/) noexcept override { return false; }
#endif

private:
    static juce::AudioProcessor::BusesProperties getDefaultBusLayout()
    {
        return juce::AudioProcessor::BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true);
    }

    void processAudioBlock (juce::AudioBuffer<float>&) override {}

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SynthBase)
};
} // namespace chowdsp
