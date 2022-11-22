#include "StatefulPlugin.h"

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
    const auto numSamples = buffer.getNumSamples();

    buffer.applyGain (0, 0, numSamples, state.params.percent->getCurrentValue());
    buffer.applyGain (1, 0, numSamples, juce::Decibels::decibelsToGain (state.params.gain->getCurrentValue()));
}

juce::AudioProcessorEditor* StatefulPlugin::createEditor()
{
    return new juce::GenericAudioProcessorEditor { *this };
}

// This creates new instances of the plugin
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new StatefulPlugin();
}
