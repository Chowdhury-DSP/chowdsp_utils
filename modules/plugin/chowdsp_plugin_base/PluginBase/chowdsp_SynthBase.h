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

        buffer.clear();

#if JUCE_MODULE_AVAILABLE_foleys_gui_magic
        this->magicState.processMidiBuffer (midi, buffer.getNumSamples(), true);
#endif

        processSynth (buffer, midi);
    }

private:
    static juce::AudioProcessor::BusesProperties getDefaultBusLayout()
    {
        return juce::AudioProcessor::BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true);
    }

    void processAudioBlock (juce::AudioBuffer<float>&) override {}
};

} // namespace chowdsp
