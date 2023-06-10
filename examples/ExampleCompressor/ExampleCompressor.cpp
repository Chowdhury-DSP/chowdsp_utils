#include "ExampleCompressor.h"
#include "PluginEditor.h"

void ExampleCompressorPlugin::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    compressor.prepare ({ sampleRate, (uint32_t) samplesPerBlock, (uint32_t) getMainBusNumInputChannels() });
}

void ExampleCompressorPlugin::processAudioBlock (juce::AudioBuffer<float>& buffer)
{
    compressor.params.attackMs = state.params.attack->getCurrentValue(),
    compressor.params.releaseMs = state.params.release->getCurrentValue(),
    compressor.params.thresholdDB = state.params.threshold->getCurrentValue(),
    compressor.params.ratio = state.params.ratio->getCurrentValue(),
    compressor.params.kneeDB = state.params.knee->getCurrentValue(),
    compressor.params.autoMakeup = state.params.autoMakeup->get(),
    compressor.levelDetector.setMode ((size_t) state.params.levelDetectorMode->getIndex());
    compressor.gainComputer.setMode ((size_t) state.params.architecture->getIndex());
    compressor.processBlock (buffer, buffer);
}

juce::AudioProcessorEditor* ExampleCompressorPlugin::createEditor()
{
    return new PluginEditor { *this };
}

// This creates new instances of the plugin
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ExampleCompressorPlugin();
}
