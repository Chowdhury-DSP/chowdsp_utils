#include "SimpleReverbPlugin.h"

namespace
{
const juce::String diffusionTimeTag = "diffuse_time";
const juce::String fdnDelayMsTag = "fdn_time";
const juce::String fdnT60LowMsTag = "fdn_t60_low";
const juce::String fdnT60HighMsTag = "fdn_t60_high";
const juce::String modAmountTag = "mod_amount";
const juce::String dryWetTag = "dry_wet";
} // namespace

SimpleReverbPlugin::SimpleReverbPlugin()
{
    diffusionTimeMsParam = vts.getRawParameterValue (diffusionTimeTag);
    fdnDelayMsParam = vts.getRawParameterValue (fdnDelayMsTag);
    fdnT60LowMsParam = vts.getRawParameterValue (fdnT60LowMsTag);
    fdnT60HighMsParam = vts.getRawParameterValue (fdnT60HighMsTag);
    modAmountParam = vts.getRawParameterValue (modAmountTag);
    dryWetParam = vts.getRawParameterValue (dryWetTag);
}

void SimpleReverbPlugin::addParameters (Parameters& params)
{
    using namespace chowdsp::ParamUtils;
    createTimeMsParameter (params, diffusionTimeTag, "Diffusion Time", createNormalisableRange (10.0f, 1000.0f, 200.0f), 300.0f);
    createTimeMsParameter (params, fdnDelayMsTag, "FDN Delay Time", createNormalisableRange (10.0f, 1000.0f, 200.0f), 300.0f);
    createTimeMsParameter (params, fdnT60LowMsTag, "FDN T60 Low", createNormalisableRange (100.0f, 10000.0f, 1000.0f), 300.0f);
    createTimeMsParameter (params, fdnT60HighMsTag, "FDN T60 High", createNormalisableRange (100.0f, 10000.0f, 1000.0f), 300.0f);
    createPercentParameter (params, modAmountTag, "Modulation", 0.0f);
    createPercentParameter (params, dryWetTag, "Dry/Wet", 0.25f);
}

void SimpleReverbPlugin::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    diffuserChain.prepare<chowdsp::Reverb::DiffuserChainEqualConfig> (sampleRate);
    fdn.prepare (sampleRate);

    diffusionTimeSmoother.reset (sampleRate, 0.05);
    fdnTimeSmoother.reset (sampleRate, 0.05);
    fdnT60LowSmoother.reset (sampleRate, 0.05);
    fdnT60HighSmoother.reset (sampleRate, 0.05);
    modAmountSmoother.reset (sampleRate, 0.05);

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
    mixer.setWetMixProportion (*dryWetParam);
    mixer.pushDrySamples (block);

    auto loadSamples = [] (float* inVec, float xL, float xR) {
        for (int i = 0; i < 8; ++i)
            inVec[i] = i % 2 == 0 ? xL : xR;
    };

    auto storeSamples = [] (const float* outVec, float& xL, float& xR) {
        xL = 0.0f;
        xR = 0.0f;
        for (int i = 0; i < 8; ++i)
        {
            xL += i % 2 == 0 ? outVec[i] : 0.0f;
            xR += i % 2 == 1 ? outVec[i] : 0.0f;
        }
    };

    auto processSample = [this, &loadSamples, &storeSamples] (float* inVec, float& xL, float& xR) {
        loadSamples (inVec, xL, xR);
        auto* diffuserOutVec = diffuserChain.process (inVec);
        auto* fdnOutVec = fdn.process (diffuserOutVec);
        storeSamples (fdnOutVec, xL, xR);
    };

    diffusionTimeSmoother.setTargetValue (*diffusionTimeMsParam);
    fdnTimeSmoother.setTargetValue (*fdnDelayMsParam);
    fdnT60LowSmoother.setTargetValue (*fdnT60LowMsParam);
    fdnT60HighSmoother.setTargetValue (*fdnT60HighMsParam);
    modAmountSmoother.setTargetValue (*modAmountParam);

    auto* dataL = buffer.getWritePointer (0);
    auto* dataR = buffer.getWritePointer (1 % numChannels);
    float inVec[8] {};

    constexpr int smallBlockSize = 16;
    for (int sampleIndex = 0; sampleIndex < numSamples; sampleIndex += smallBlockSize)
    {
        const auto samplesToProcess = juce::jmin (smallBlockSize, numSamples - sampleIndex);

        diffuserChain.setDiffusionTimeMs (diffusionTimeSmoother.getNextValue());

        const auto modAmountMult = modAmountSmoother.skip (samplesToProcess);
        for (float& lfoVal : lfoVals)
            lfoVal = lfoVal * modAmountMult * 0.01f + 1.0f;
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
    return new juce::GenericAudioProcessorEditor (*this);
}

// This creates new instances of the plugin
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SimpleReverbPlugin();
}
