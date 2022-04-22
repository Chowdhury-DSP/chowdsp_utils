#pragma once

#include <chowdsp_dsp/chowdsp_dsp.h>

#include "EQParams.h"

/** "Prototype" EQ from which the linear phase EQ will be constructed. */
class PrototypeEQ
{
public:
    PrototypeEQ() = default;

    using Params = EQParams;

    void setParameters (const Params& params);

    void prepare (const juce::dsp::ProcessSpec& spec);

    void reset();

    void processBlock (juce::AudioBuffer<float>& buffer);

private:
    using EQBand = chowdsp::EQBand<float,
                                   chowdsp::FirstOrderHPF<float>,
                                   chowdsp::SecondOrderHPF<float>,
                                   chowdsp::PeakingFilter<float>,
                                   chowdsp::LowShelfFilter<float>,
                                   chowdsp::HighShelfFilter<float>,
                                   chowdsp::FirstOrderLPF<float>,
                                   chowdsp::SecondOrderLPF<float>>;
    std::array<EQBand, Params::numBands> bands;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PrototypeEQ)
};
