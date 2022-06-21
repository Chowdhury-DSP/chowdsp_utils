#include "AutoWahPlugin.h"

namespace
{
const juce::String freqTag = "freq";
const juce::String qValTag = "q_val";
const juce::String gainTag = "gain";
const juce::String attackTag = "attack";
const juce::String releaseTag = "release";
const juce::String freqModTag = "freq_mod";
} // namespace

AutoWahPlugin::AutoWahPlugin()
{
    freqHzParam = vts.getRawParameterValue (freqTag);
    qValParam = vts.getRawParameterValue (qValTag);
    gainDBParam = vts.getRawParameterValue (gainTag);
    attackMsParam = vts.getRawParameterValue (attackTag);
    releaseMsParam = vts.getRawParameterValue (releaseTag);
    freqModParam = vts.getRawParameterValue (freqModTag);
}

void AutoWahPlugin::addParameters (Parameters& params)
{
    using namespace chowdsp::ParamUtils;

    createFreqParameter (params, freqTag, "Frequency", 200.0f, 2000.0f, 500.0f, 500.0f);
    emplace_param<chowdsp::FloatParameter> (params, qValTag, "Q", createNormalisableRange (1.0f, 15.0f, 5.0f), 5.0f, &floatValToString, &stringToFloatVal);
    createGainDBParameter (params, gainTag, "Gain", 0.0f, 12.0f, 0.0f);

    createTimeMsParameter (params, attackTag, "Attack", createNormalisableRange (0.1f, 10.0f, 1.0f), 1.0f);
    createTimeMsParameter (params, releaseTag, "Release", createNormalisableRange (2.5f, 250.0f, 50.0f), 50.0f);

    createPercentParameter (params, freqModTag, "Freq. Mod", 0.5f);
}

void AutoWahPlugin::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    const auto&& spec = juce::dsp::ProcessSpec { sampleRate, (juce::uint32) samplesPerBlock, (juce::uint32) getTotalNumInputChannels() };

    levelDetector.prepare (spec);
    levelBuffer.setSize (1, samplesPerBlock);

    wahFilter.prepare (spec);
    fs = (float) sampleRate;
}

void AutoWahPlugin::processAudioBlock (juce::AudioBuffer<float>& buffer)
{
    const auto numSamples = buffer.getNumSamples();
    auto&& block = juce::dsp::AudioBlock<float> { buffer };

    levelBuffer.setSize (1, numSamples, false, false, true);
    auto&& levelBlock = juce::dsp::AudioBlock<float> { levelBuffer };
    levelDetector.setParameters (*attackMsParam, *releaseMsParam);
    levelDetector.process (juce::dsp::ProcessContextNonReplacing<float> { block, levelBlock });
    const auto* levelData = levelBuffer.getReadPointer (0);

    const auto baseFreqHz = freqHzParam->load();
    const auto curQVal = qValParam->load();
    const auto curGain = juce::Decibels::decibelsToGain (gainDBParam->load());
    const auto curFreqMod = 9.0f * *freqModParam;
    wahFilter.process (
        juce::dsp::ProcessContextReplacing<float> { block },
        [&] (size_t sampleIndex) {
            const auto curFreqHz = baseFreqHz + baseFreqHz * curFreqMod * levelData[sampleIndex];
            wahFilter.calcCoefs (curFreqHz, curQVal, curGain, fs);
        });
}

juce::AudioProcessorEditor* AutoWahPlugin::createEditor()
{
    return new juce::GenericAudioProcessorEditor (*this);
}

// This creates new instances of the plugin
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AutoWahPlugin();
}
