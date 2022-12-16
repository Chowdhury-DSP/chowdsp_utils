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
    using namespace chowdsp::ParamUtils;
    loadParameterPointer (freqHzParam, vts, freqTag);
    loadParameterPointer (qValParam, vts, qValTag);
    loadParameterPointer (gainDBParam, vts, gainTag);
    loadParameterPointer (attackMsParam, vts, attackTag);
    loadParameterPointer (releaseMsParam, vts, releaseTag);
    loadParameterPointer (freqModParam, vts, freqModTag);
}

void AutoWahPlugin::addParameters (Parameters& params)
{
    using namespace chowdsp::ParamUtils;

    createFreqParameter (params, { freqTag, 100 }, "Frequency", 200.0f, 2000.0f, 500.0f, 500.0f);
    emplace_param<chowdsp::FloatParameter> (params, chowdsp::ParameterID { qValTag, 100 }, "Q", createNormalisableRange (1.0f, 15.0f, 5.0f), 5.0f, &floatValToString, &stringToFloatVal);
    createGainDBParameter (params, { gainTag, 100 }, "Gain", 0.0f, 12.0f, 0.0f);

    createTimeMsParameter (params, { attackTag, 100 }, "Attack", createNormalisableRange (0.1f, 10.0f, 1.0f), 1.0f);
    createTimeMsParameter (params, { releaseTag, 100 }, "Release", createNormalisableRange (2.5f, 250.0f, 50.0f), 50.0f);

    createPercentParameter (params, { freqModTag, 100 }, "Freq. Mod", 0.5f);
}

void AutoWahPlugin::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    const auto&& spec = juce::dsp::ProcessSpec { sampleRate, (juce::uint32) samplesPerBlock, (juce::uint32) getTotalNumInputChannels() };

    levelDetector.prepare (spec);
    levelBuffer.setMaxSize (1, samplesPerBlock);

    wahFilter.prepare (spec);
    fs = (float) sampleRate;
}

void AutoWahPlugin::processAudioBlock (juce::AudioBuffer<float>& buffer)
{
    const auto numSamples = buffer.getNumSamples();
    auto&& block = juce::dsp::AudioBlock<float> { buffer };

    levelBuffer.setCurrentSize (1, numSamples);
    levelDetector.setParameters (*attackMsParam, *releaseMsParam);
    levelDetector.processBlock (buffer, levelBuffer);
    const auto* levelData = levelBuffer.getReadPointer (0);

    const auto baseFreqHz = freqHzParam->getCurrentValue();
    const auto curQVal = qValParam->getCurrentValue();
    const auto curGain = juce::Decibels::decibelsToGain (gainDBParam->getCurrentValue());
    const auto curFreqMod = 9.0f * *freqModParam;
    wahFilter.process (
        juce::dsp::ProcessContextReplacing<float> { block },
        [&] (size_t sampleIndex)
        {
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
