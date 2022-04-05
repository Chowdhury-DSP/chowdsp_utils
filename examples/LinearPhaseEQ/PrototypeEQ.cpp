#include "PrototypeEQ.h"

void PrototypeEQ::setParameters (const Params& params, bool force)
{
    auto setParam = [force] (auto& smoother, float newValue) {
        if (force)
            smoother.setCurrentAndTargetValue (newValue);
        else
            smoother.setTargetValue (newValue);
    };

    setParam (lowCutFreqHzSmooth, params.lowCutFreqHz);
    setParam (lowCutQSmooth, params.lowCutQ);
    setParam (peakingFilterFreqHzSmooth, params.peakingFilterFreqHz);
    setParam (peakingFilterQSmooth, params.peakingFilterQ);
    setParam (peakingFilterGainSmooth, juce::Decibels::decibelsToGain (params.peakingFilterGainDB));
    setParam (highCutFreqHzSmooth, juce::jmin (params.highCutFreqHz, 0.4995f * fs));
    setParam (highCutQSmooth, params.highCutQ);
}

void PrototypeEQ::prepare (const juce::dsp::ProcessSpec& spec)
{
    jassert (spec.numChannels == 1); // only set up to handle mono signals (for now)

    fs = (float) spec.sampleRate;

    for (auto* smoother : { &lowCutFreqHzSmooth, &lowCutQSmooth, &peakingFilterFreqHzSmooth, &peakingFilterQSmooth, &peakingFilterGainSmooth, &highCutFreqHzSmooth, &highCutQSmooth })
        smoother->reset (spec.sampleRate, 0.05);

    reset();
}

void PrototypeEQ::reset()
{
    lowCutFilter.reset();
    peakingFilter.reset();
    highCutFilter.reset();
}

void PrototypeEQ::processBlock (juce::AudioBuffer<float>& buffer)
{
    processHighCut (buffer);
    processPeaking (buffer);
    processLowCut (buffer);
}

template <typename FilterType, typename ParamUpdater>
void processFilter (juce::AudioBuffer<float>& buffer, FilterType& filter, ParamUpdater&& updater, bool isSmoothing)
{
    auto* x = buffer.getWritePointer (0);
    const auto numSamples = buffer.getNumSamples();

    if (! isSmoothing)
    {
        updater();
        filter.processBlock (x, numSamples);
    }
    else
    {
        for (int n = 0; n < numSamples; ++n)
        {
            updater();
            x[n] = filter.processSample (x[n]);
        }
    }
}

void PrototypeEQ::processHighCut (juce::AudioBuffer<float>& buffer)
{
    const auto isSmoothing = highCutFreqHzSmooth.isSmoothing() || highCutQSmooth.isSmoothing();
    processFilter (
        buffer, highCutFilter, [this] { highCutFilter.calcCoefs (highCutFreqHzSmooth.getNextValue(), highCutQSmooth.getNextValue(), fs); }, isSmoothing);
}

void PrototypeEQ::processPeaking (juce::AudioBuffer<float>& buffer)
{
    const auto isSmoothing = peakingFilterFreqHzSmooth.isSmoothing() || peakingFilterQSmooth.isSmoothing() || peakingFilterGainSmooth.isSmoothing();
    processFilter (
        buffer, peakingFilter, [this] { peakingFilter.calcCoefs (peakingFilterFreqHzSmooth.getNextValue(), peakingFilterQSmooth.getNextValue(), peakingFilterGainSmooth.getNextValue(), fs); }, isSmoothing);
}

void PrototypeEQ::processLowCut (juce::AudioBuffer<float>& buffer)
{
    const auto isSmoothing = lowCutFreqHzSmooth.isSmoothing() || lowCutQSmooth.isSmoothing();
    processFilter (
        buffer, lowCutFilter, [this] { lowCutFilter.calcCoefs (lowCutFreqHzSmooth.getNextValue(), lowCutQSmooth.getNextValue(), fs); }, isSmoothing);
}
