#include "SignalGeneratorPlugin.h"

namespace
{
const juce::String freqTag = "freq_hz";
const juce::String typeTag = "tone_type";
const juce::String upsampleTag = "upsample_tag";
const juce::String gainTag = "gain_db";
const juce::String waveshaperTag = "waveshaper";
} // namespace

SignalGeneratorPlugin::SignalGeneratorPlugin()
{
    using namespace chowdsp::ParamUtils;
    freqHzParamSmoothed.setParameterHandle (getParameterPointer<chowdsp::FloatParameter*> (vts, freqTag));
    loadParameterPointer (toneTypeParam, vts, typeTag);
    loadParameterPointer (upSampleParam, vts, upsampleTag);
    loadParameterPointer (gainDBParam, vts, gainTag);
    loadParameterPointer (waveshaperParam, vts, waveshaperTag);
}

void SignalGeneratorPlugin::addParameters (Parameters& params)
{
    using namespace chowdsp::ParamUtils;
    createFreqParameter (params, { freqTag, 100 }, "Frequency", 50.0f, 50000.0f, 2500.0f, 1000.0f);
    createGainDBParameter (params, { gainTag, 100 }, "Gain", -45.0f, 12.0f, -24.0f);
    emplace_param<chowdsp::ChoiceParameter> (params, chowdsp::ParameterID { typeTag, 100 }, "Tone Type", juce::StringArray { "Sine", "Saw", "Square", "Triangle" }, 0);
    emplace_param<chowdsp::ChoiceParameter> (params, chowdsp::ParameterID { upsampleTag, 100 }, "Upsample", juce::StringArray { "1x", "2x", "3x", "4x" }, 0);

    juce::StringArray waveshapeOptions { "None", "Hard Clip", "Tanh Clip", "Cubic Clip", "9th-Order Clip", "Full Wave Rectify", "West Coast", "Wave Multiply" };
    emplace_param<chowdsp::ChoiceParameter> (params, chowdsp::ParameterID { waveshaperTag, 100 }, "Waveshaper", waveshapeOptions, 0);
}

void SignalGeneratorPlugin::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    setRateAndBufferSizeDetails (sampleRate, samplesPerBlock);

    gain.setRampDurationSeconds (0.01);
    prepareTones (sampleRate, samplesPerBlock);

    const auto spec = juce::dsp::ProcessSpec { sampleRate, (juce::uint32) samplesPerBlock, (juce::uint32) getMainBusNumInputChannels() };
    adaaHardClipper.prepare ((int) spec.numChannels);
    adaaTanhClipper.prepare ((int) spec.numChannels);
    adaaCubicClipper.prepare ((int) spec.numChannels);
    adaa9thOrderClipper.prepare ((int) spec.numChannels);
    fullWaveRectifier.prepare ((int) spec.numChannels);
    westCoastFolder.prepare ((int) spec.numChannels);
    waveMultiplyFolder.prepare ((int) spec.numChannels);

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
    previousUpSampleChoice = -1;

    upsampledBuffer.setSize ((int) spec.numChannels, 4 * samplesPerBlock);
}

void SignalGeneratorPlugin::prepareTones (double sampleRate, int maxSamplesPerBlock)
{
    const auto spec = juce::dsp::ProcessSpec { sampleRate, (juce::uint32) maxSamplesPerBlock, (juce::uint32) getMainBusNumInputChannels() };
    sine.prepare (spec);
    saw.prepare (spec);
    square.prepare (spec);
    triangle.prepare (spec);

    gain.prepare (spec);

    freqHzParamSmoothed.prepare (sampleRate, maxSamplesPerBlock);
    freqHzParamSmoothed.setRampLength (0.05);
    freqHzParamSmoothed.mappingFunction = [fs = (float) sampleRate] (auto targetFrequency)
    {
        return juce::jmin (targetFrequency, 0.48f * fs);
    };
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

    auto processTone = [this, &block, numChannels = buffer.getNumChannels(), numSamples = buffer.getNumSamples()] (auto& tone)
    {
        if (resampler == nullptr)
            upsampledBuffer.setSize (numChannels, numSamples);
        else
            upsampledBuffer.setSize (numChannels, resampler->getDownsamplingRatio() * numSamples);
        upsampledBuffer.clear();

        auto&& upsampledBlock = juce::dsp::AudioBlock<float> { upsampledBuffer };
        auto&& upsampledContext = juce::dsp::ProcessContextReplacing<float> { upsampledBlock };

        freqHzParamSmoothed.process (upsampledBuffer.getNumSamples());
        if (! freqHzParamSmoothed.isSmoothing())
        {
            auto targetFrequency = freqHzParamSmoothed.getCurrentValue();
            tone.setFrequency (targetFrequency);
            tone.processBlock (upsampledBuffer);
        }
        else
        {
            const auto osNumSamples = upsampledBuffer.getNumSamples();
            const auto* freqHzData = freqHzParamSmoothed.getSmoothedBuffer();
            auto* data = upsampledBuffer.getWritePointer (0);

            for (int n = 0; n < osNumSamples; ++n)
            {
                tone.setFrequency (freqHzData[n]);
                data[n] = tone.processSample();
            }

            for (int ch = 1; ch < upsampledBuffer.getNumChannels(); ++ch)
                upsampledBuffer.copyFrom (ch, 0, upsampledBuffer, 0, 0, osNumSamples);
        }

        gain.setGainDecibels (gainDBParam->getCurrentValue());
        gain.process (upsampledBuffer);

        if (waveshaperParam->getIndex() == 0)
        {
            // no waveshaper
        }
        else if (waveshaperParam->getIndex() == 1)
        {
            adaaHardClipper.process (upsampledContext);
        }
        else if (waveshaperParam->getIndex() == 2)
        {
            adaaTanhClipper.process (upsampledContext);
        }
        else if (waveshaperParam->getIndex() == 3)
        {
            adaaCubicClipper.process (upsampledContext);
        }
        else if (waveshaperParam->getIndex() == 4)
        {
            adaa9thOrderClipper.process (upsampledContext);
        }
        else if (waveshaperParam->getIndex() == 5)
        {
            fullWaveRectifier.process (upsampledContext);
        }
        else if (waveshaperParam->getIndex() == 6)
        {
            westCoastFolder.process (upsampledContext);
        }
        else if (waveshaperParam->getIndex() == 7)
        {
            waveMultiplyFolder.processBlock (upsampledBuffer);
        }

        if (resampler == nullptr)
        {
            block.copyFrom (upsampledBlock);
        }
        else
        {
            auto&& bufferView = chowdsp::BufferView<float> { block };
            chowdsp::BufferMath::copyBufferData (resampler->process (upsampledBlock), bufferView);
        }
    };

    setUpSampleChoice();
    const auto typeChoice = (int) *toneTypeParam;
    if (typeChoice == 0)
        processTone (sine);
    else if (typeChoice == 1)
        processTone (saw);
    else if (typeChoice == 2)
        processTone (square);
    else if (typeChoice == 3)
        processTone (triangle);
    else
        jassertfalse; // unknown type!
}

juce::AudioProcessorEditor* SignalGeneratorPlugin::createEditor()
{
    return new juce::GenericAudioProcessorEditor (*this);
}

#if ! CHOWDSP_BUILDING_FORWARDING_TEST_PLUGIN
// This creates new instances of the plugin
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SignalGeneratorPlugin();
}
#endif
