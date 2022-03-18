#pragma once

#include <chowdsp_dsp/chowdsp_dsp.h>

#include "EQParams.h"

/** "Prototype" EQ from which the linear phase EQ will be constructed. */
class PrototypeEQ
{
public:
    PrototypeEQ() = default;

    using Params = EQParams;

    void setParameters (const Params& params, bool force = false);

    void prepare (const juce::dsp::ProcessSpec& spec);

    void reset();

    void processBlock (juce::AudioBuffer<float>& buffer);

private:
    void processHighCut (juce::AudioBuffer<float>& buffer);
    void processPeaking (juce::AudioBuffer<float>& buffer);
    void processLowCut (juce::AudioBuffer<float>& buffer);

    chowdsp::SecondOrderHPF<float> lowCutFilter;
    chowdsp::PeakingFilter<float> peakingFilter;
    chowdsp::SecondOrderLPF<float> highCutFilter;

    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Multiplicative> lowCutFreqHzSmooth;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Multiplicative> lowCutQSmooth;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Multiplicative> peakingFilterFreqHzSmooth;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Multiplicative> peakingFilterQSmooth;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Multiplicative> peakingFilterGainSmooth;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Multiplicative> highCutFreqHzSmooth;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Multiplicative> highCutQSmooth;

    float fs = 48000.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PrototypeEQ)
};
