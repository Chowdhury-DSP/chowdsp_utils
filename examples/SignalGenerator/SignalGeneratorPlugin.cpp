#include <chowdsp_gui/chowdsp_gui.h>
#include "SignalGeneratorPlugin.h"

SignalGeneratorPlugin::SignalGeneratorPlugin()
{
    freqHzParamSmoothed.setParameterHandle (state.params.freqParam.get());

    float additiveHarmonics[numHarmonics] {};
    for (auto [i, amp] : chowdsp::enumerate (additiveHarmonics))
        amp = 1.0f / float (i + 1);
    additiveSaw.setHarmonicAmplitudes (additiveHarmonics);
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
    adaaSineClipper.prepare ((int) spec.numChannels);
    clipGuard.prepare (spec);

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
    additiveSaw.prepare ((float) sampleRate);

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
    const auto upsampleChoice = state.params.upsampleParam->getIndex();
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

        gain.setGainDecibels (state.params.gainParam->getCurrentValue());
        gain.process (upsampledBuffer);

        const auto waveshapeIndex = state.params.waveshapeParam->getIndex();
        if (waveshapeIndex == 0)
        {
            // no waveshaper
            clipGuard.setCeiling (100.0f);
        }
        else if (waveshapeIndex == 1)
        {
            adaaHardClipper.process (upsampledContext);
            clipGuard.setCeiling (1.0f);
        }
        else if (waveshapeIndex == 2)
        {
            adaaTanhClipper.process (upsampledContext);
            clipGuard.setCeiling (1.0f);
        }
        else if (waveshapeIndex == 3)
        {
            adaaCubicClipper.process (upsampledContext);
            clipGuard.setCeiling (1.0f);
        }
        else if (waveshapeIndex == 4)
        {
            adaa9thOrderClipper.process (upsampledContext);
            clipGuard.setCeiling (1.0f);
        }
        else if (waveshapeIndex == 5)
        {
            fullWaveRectifier.process (upsampledContext);
            clipGuard.setCeiling (100.0f);
        }
        else if (waveshapeIndex == 6)
        {
            westCoastFolder.process (upsampledContext);
            clipGuard.setCeiling (100.0f);
        }
        else if (waveshapeIndex == 7)
        {
            waveMultiplyFolder.processBlock (upsampledBuffer);
            clipGuard.setCeiling (100.0f);
        }
        else if (waveshapeIndex == 8)
        {
            adaaSineClipper.process (upsampledContext);
            clipGuard.setCeiling (100.0f);
        }

        if (resampler == nullptr)
        {
            block.copyFrom (upsampledBlock);
        }
        else
        {
            resampler->process (upsampledBuffer, block);
        }

        clipGuard.processBlock (block);
    };

    setUpSampleChoice();
    const auto typeChoice = state.params.typeParam->getIndex();
    if (typeChoice == 0)
        processTone (sine);
    else if (typeChoice == 1)
        processTone (saw);
    else if (typeChoice == 2)
        processTone (square);
    else if (typeChoice == 3)
        processTone (triangle);
    else if (typeChoice == 4)
        processTone (additiveSaw);
    else
        jassertfalse; // unknown type!
}

juce::AudioProcessorEditor* SignalGeneratorPlugin::createEditor()
{
    return new chowdsp::ParametersViewEditor { *this };
}

#if ! CHOWDSP_BUILDING_FORWARDING_TEST_PLUGIN
// This creates new instances of the plugin
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SignalGeneratorPlugin();
}
#endif
