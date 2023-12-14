#include "ForwardingTestPlugin.h"
#include "PluginEditor.h"

ForwardingTestPlugin::ForwardingTestPlugin() : forwardingParameters (*this, state)
{
    processorChangedCallback = state.addParameterListener (
        state.params.processorChoice,
        chowdsp::ParameterListenerThread::MessageThread,
        [this]
        {
            if (auto* newProcessor = getProcessorForIndex (state.params.processorChoice->getIndex()))
            {
                forwardingParameters.setParameterRange (0,
                                                        numForwardParameters,
                                                        [&processorParameters = newProcessor->getParameters()] (int i) -> chowdsp::ParameterForwardingInfo
                                                        {
                                                            if (auto* paramCast = dynamic_cast<juce::RangedAudioParameter*> (processorParameters[i]))
                                                                return { paramCast, paramCast->name };

                                                            return {};
                                                        });
            }
            else
            {
                forwardingParameters.clearParameterRange (0, numForwardParameters);
            }
        });
}

ForwardingTestPlugin::~ForwardingTestPlugin() = default;

void ForwardingTestPlugin::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    toneGenerator.prepareToPlay (sampleRate, samplesPerBlock);
    reverb.prepareToPlay (sampleRate, samplesPerBlock);
    wernerFilter.prepareToPlay (sampleRate, samplesPerBlock);
    arpFilter.prepareToPlay (sampleRate, samplesPerBlock);
    polygonalOsc.prepareToPlay (sampleRate, samplesPerBlock);
    bandSplit.prepareToPlay (sampleRate, samplesPerBlock);
    plateReverb.prepareToPlay (sampleRate, samplesPerBlock);
    polyphaseOversampling.prepareToPlay (sampleRate, samplesPerBlock);
}

void ForwardingTestPlugin::processAudioBlock (juce::AudioBuffer<float>& buffer)
{
    juce::MidiBuffer midi {};
    if (auto* processor = getProcessorForIndex (state.params.processorChoice->getIndex()))
    {
        processor->processBlock (buffer, midi);
    }
}

juce::AudioProcessorEditor* ForwardingTestPlugin::createEditor()
{
    return new PluginEditor (*this);
}

juce::AudioProcessor* ForwardingTestPlugin::getProcessorForIndex (int index)
{
    if (index == 1)
        return &toneGenerator;

    if (index == 2)
        return &reverb;

    if (index == 3)
        return &wernerFilter;

    if (index == 4)
        return &arpFilter;

    if (index == 5)
        return &polygonalOsc;

    if (index == 6)
        return &bandSplit;

    if (index == 7)
        return &plateReverb;

    if (index == 8)
        return &polyphaseOversampling;

    return nullptr;
}

// This creates new instances of the plugin
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ForwardingTestPlugin();
}
