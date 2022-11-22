#include "StatefulPlugin.h"
#include "PluginEditor.h"

StatefulPlugin::StatefulPlugin()
{
    juce::ignoreUnused (this);
}

void StatefulPlugin::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::ignoreUnused (sampleRate, samplesPerBlock);
}

void StatefulPlugin::processAudioBlock (juce::AudioBuffer<float>& buffer)
{
    if (! state.params.onOff->get())
        return;

    const auto numSamples = buffer.getNumSamples();

    float leftGain = 1.0f, rightGain = 1.0f;

    const auto mode = state.params.mode->getIndex();
    if (mode == 0)
    {
        leftGain = rightGain = state.params.percent->getCurrentValue();
    }
    else if (mode == 1)
    {
        leftGain = rightGain = juce::Decibels::decibelsToGain (state.params.gain->getCurrentValue());
    }
    else if (mode == 2)
    {
        leftGain = state.params.percent->getCurrentValue();
        rightGain = juce::Decibels::decibelsToGain (state.params.gain->getCurrentValue());
    }
    else if (mode == 3)
    {
        rightGain = state.params.percent->getCurrentValue();
        leftGain = juce::Decibels::decibelsToGain (state.params.gain->getCurrentValue());
    }

    buffer.applyGain (0, 0, numSamples, leftGain);
    buffer.applyGain (1, 0, numSamples, rightGain);
}

juce::AudioProcessorEditor* StatefulPlugin::createEditor()
{
    return new PluginEditor { *this };
}

// This creates new instances of the plugin
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new StatefulPlugin();
}
