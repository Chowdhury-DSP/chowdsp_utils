#include <chowdsp_gui/chowdsp_gui.h>
#include "SimpleReverbPlugin.h"

SimpleReverbPlugin::SimpleReverbPlugin() = default;

void SimpleReverbPlugin::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    fdn.prepare (sampleRate);

    diffusionTimeSmoother.reset (sampleRate, 0.05);
    fdnTimeSmoother.reset (sampleRate, 0.05);
    fdnT60LowSmoother.reset (sampleRate, 0.05);
    fdnT60HighSmoother.reset (sampleRate, 0.05);
    modAmountSmoother.reset (sampleRate, 0.05);

    diffuser.prepare ({ sampleRate, (juce::uint32) samplesPerBlock, 2 });

    for (auto& lfo : lfos)
        lfo.prepare ({ sampleRate, (juce::uint32) samplesPerBlock, 1 });
    lfos[0].setFrequency (2.0f);
    lfos[1].setFrequency (0.95f);

    mixer.prepare ({ sampleRate, (juce::uint32) samplesPerBlock, (juce::uint32) getMainBusNumInputChannels() });
}

void SimpleReverbPlugin::processAudioBlock (juce::AudioBuffer<float>& buffer)
{
    const auto numChannels = buffer.getNumChannels();
    const auto numSamples = buffer.getNumSamples();

    auto&& block = juce::dsp::AudioBlock<float> { buffer };
    mixer.setWetMixProportion (*state.params.dryWet);
    mixer.pushDrySamples (block);

    diffuser.setDiffusionTime (state.params.diffusionTime->getCurrentValue() * 0.001f);
    diffuser.processBlock (buffer);

    auto loadSamples = [] (float* inVec, float xL, float xR)
    {
        for (int i = 0; i < 8; i += 2)
        {
            inVec[i] = xL;
            inVec[i + 1] = xR;
        }
    };

    auto storeSamples = [] (const float* outVec, float& xL, float& xR)
    {
        xL = 0.0f;
        xR = 0.0f;
        for (int i = 0; i < 8; i += 2)
        {
            xL += outVec[i];
            xR += outVec[i + 1];
        }

        xL *= 1.0f / 8.0f;
        xR *= 1.0f / 8.0f;
    };

    auto processSample = [this, &loadSamples, &storeSamples] (float* inVec, float& xL, float& xR)
    {
        loadSamples (inVec, xL, xR);
        auto* fdnOutVec = fdn.process (inVec);
        juce::ignoreUnused (fdnOutVec, storeSamples);
        storeSamples (fdnOutVec, xL, xR);
    };

    diffusionTimeSmoother.setTargetValue (*state.params.diffusionTime);
    fdnTimeSmoother.setTargetValue (*state.params.fdnDelay);
    fdnT60LowSmoother.setTargetValue (*state.params.fdnT60Low);
    fdnT60HighSmoother.setTargetValue (*state.params.fdnT60High);
    modAmountSmoother.setTargetValue (*state.params.modAmount);

    auto* dataL = buffer.getWritePointer (0);
    auto* dataR = buffer.getWritePointer (1 % numChannels);
    float inVec[8] {};

    constexpr int smallBlockSize = 16;
    for (int sampleIndex = 0; sampleIndex < numSamples; sampleIndex += smallBlockSize)
    {
        const auto samplesToProcess = juce::jmin (smallBlockSize, numSamples - sampleIndex);

        const auto modAmountMult = modAmountSmoother.skip (samplesToProcess);
        for (float& lfoVal : lfoVals)
            lfoVal = lfoVal * modAmountMult * 0.25f + 1.0f;
        fdn.setDelayTimeMsWithModulators<2> (fdnTimeSmoother.skip (samplesToProcess), lfoVals);

        fdn.getFDNConfig().setDecayTimeMs (fdn, fdnT60LowSmoother.skip (samplesToProcess), fdnT60HighSmoother.skip (samplesToProcess), 1000.0f);

        for (int n = sampleIndex; n < sampleIndex + samplesToProcess; ++n)
        {
            for (int i = 0; i < 2; ++i)
                lfoVals[i] = lfos[i].processSample();

            processSample (inVec, dataL[n], dataR[n]);
        }
    }

    mixer.mixWetSamples (block);
}

juce::AudioProcessorEditor* SimpleReverbPlugin::createEditor()
{
    return new chowdsp::ParametersViewEditor { *this };
}

#if ! CHOWDSP_BUILDING_FORWARDING_TEST_PLUGIN
// This creates new instances of the plugin
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SimpleReverbPlugin();
}
#endif
