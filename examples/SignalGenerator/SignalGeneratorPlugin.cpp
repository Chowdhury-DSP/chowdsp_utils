#include "SignalGeneratorPlugin.h"

namespace
{
const juce::String freqTag = "freq_hz";
const juce::String typeTag = "tone_type";
const juce::String upsampleTag = "upsample_tag";
const juce::String gainTag = "gain_db";
} // namespace

SignalGeneratorPlugin::SignalGeneratorPlugin()
{
    freqHzParam = vts.getRawParameterValue (freqTag);
    toneTypeParam = vts.getRawParameterValue (typeTag);
    upSampleParam = vts.getRawParameterValue (upsampleTag);
    gainDBParam = vts.getRawParameterValue (gainTag);
}

void SignalGeneratorPlugin::addParameters (Parameters& params)
{
    using namespace chowdsp::ParamUtils;
    createFreqParameter (params, freqTag, "Frequency", 50.0f, 50000.0f, 2500.0f, 1000.0f);
    createGainDBParameter (params, gainTag, "Gain", -45.0f, 6.0f, -24.0f);
    emplace_param<chowdsp::ChoiceParameter> (params, typeTag, "Tone Type", juce::StringArray { "Sine", "Saw", "Square" }, 0);
    emplace_param<chowdsp::ChoiceParameter> (params, upsampleTag, "Upsample", juce::StringArray { "1x", "2x", "3x", "4x" }, 0);
}

void SignalGeneratorPlugin::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    setRateAndBufferSizeDetails (sampleRate, samplesPerBlock);

    prepareTones (sampleRate, samplesPerBlock);

    const auto spec = juce::dsp::ProcessSpec { sampleRate, (juce::uint32) samplesPerBlock, (juce::uint32) getMainBusNumInputChannels() };
    gain.prepare (spec);
    gain.setRampDurationSeconds (0.01);

    int resampleRatio = 2;
    for (auto* r : { &resample2, &resample3, &resample4 })
    {
        auto osSpec = spec;
        osSpec.sampleRate *= resampleRatio;
        osSpec.maximumBlockSize *= (juce::uint32) resampleRatio;
        r->prepare (osSpec, resampleRatio);
        resampleRatio += 1;
    }

    resampler = nullptr;
    previousUpSampleChoice = 0;

    upsampledBuffer.setSize ((int) spec.numChannels, 4 * samplesPerBlock);
}

void SignalGeneratorPlugin::prepareTones (double sampleRate, int maxSamplesPerBlock)
{
    const auto spec = juce::dsp::ProcessSpec { sampleRate, (juce::uint32) maxSamplesPerBlock, (juce::uint32) getMainBusNumInputChannels() };
    sine.prepare (spec);
    saw.prepare (spec);
    square.prepare (spec);
}

void SignalGeneratorPlugin::setUpSampleChoice()
{
    const auto upsampleChoice = (int) *upSampleParam;
    if (upsampleChoice != previousUpSampleChoice)
    {
        previousUpSampleChoice = upsampleChoice;
        auto sampleRate = getSampleRate();
        auto curBlockSize = getBlockSize();

        if (upsampleChoice == 0)
        {
            resampler = nullptr;
            prepareTones (sampleRate, curBlockSize);
        }
        else if (upsampleChoice == 1)
        {
            resampler = &resample2;
            prepareTones (2 * sampleRate, 2 * curBlockSize);
        }
        else if (upsampleChoice == 2)
        {
            resampler = &resample3;
            prepareTones (3 * sampleRate, 3 * curBlockSize);
        }
        else if (upsampleChoice == 3)
        {
            resampler = &resample4;
            prepareTones (4 * sampleRate, 4 * curBlockSize);
        }
        else
        {
            jassertfalse; // unknown upsample amount!
        }
    }
}

void SignalGeneratorPlugin::processAudioBlock (juce::AudioBuffer<float>& buffer)
{
    buffer.clear();
    auto&& block = juce::dsp::AudioBlock<float> { buffer };

    auto processTone = [this, &block, numChannels = buffer.getNumChannels(), numSamples = buffer.getNumSamples()] (auto& tone) {
        auto targetFrequency = freqHzParam->load();
        if (targetFrequency > 0.48f * (float) getSampleRate())
        {
            tone.reset();
            targetFrequency = 0.0f;
        }
        tone.setFrequency (targetFrequency);

        if (resampler == nullptr)
            upsampledBuffer.setSize (numChannels, numSamples);
        else
            upsampledBuffer.setSize (numChannels, resampler->getDownsamplingRatio() * numSamples);
        upsampledBuffer.clear();

        auto&& upsampledBlock = juce::dsp::AudioBlock<float> { upsampledBuffer };
        tone.process (juce::dsp::ProcessContextReplacing<float> { upsampledBlock });

        if (resampler == nullptr)
            block.copyFrom (upsampledBlock);
        else
            block.copyFrom (resampler->process (upsampledBlock));
    };

    setUpSampleChoice();
    const auto typeChoice = (int) *toneTypeParam;
    if (typeChoice == 0)
        processTone (sine);
    else if (typeChoice == 1)
        processTone (saw);
    else if (typeChoice == 2)
        processTone (square);
    else
        jassertfalse; // unknown type!

    gain.setGainDecibels (*gainDBParam);
    gain.process (juce::dsp::ProcessContextReplacing<float> { block });
}

juce::AudioProcessorEditor* SignalGeneratorPlugin::createEditor()
{
    return new juce::GenericAudioProcessorEditor (*this);
}

// This creates new instances of the plugin
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SignalGeneratorPlugin();
}
