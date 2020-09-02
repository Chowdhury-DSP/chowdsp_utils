#pragma once

namespace chowdsp
{
    
template<class Processor>
class SynthBase : public PluginBase<Processor>
{
public:
    SynthBase() = default;

    bool acceptsMidi() const override { return false; }

    virtual void processSynth (juce::AudioBuffer<float>&, juce::MidiBuffer&) = 0;

    void processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi) override
    {
        juce::ScopedNoDenormals noDenormals;

        buffer.clear();

        this->magicState.processMidiBuffer (midi, buffer.getNumSamples(), true);

        processSynth (buffer, midi);
    }

private:
    void processBlock (juce::AudioBuffer<float>&) override {}
};
    
} // namespace chowdsp

