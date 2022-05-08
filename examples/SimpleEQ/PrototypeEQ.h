#pragma once

#include <chowdsp_eq/chowdsp_eq.h>

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
                                   chowdsp::NthOrderFilter<float, 4, chowdsp::StateVariableFilterType::Highpass>,
                                   chowdsp::ButterworthFilter<8, chowdsp::ButterworthFilterType::Highpass>,
                                   chowdsp::ChebyshevIIFilter<8, chowdsp::ChebyshevFilterType::Highpass>,
                                   chowdsp::EllipticFilter<8, chowdsp::EllipticFilterType::Highpass>,
                                   chowdsp::LowShelfFilter<float>,
                                   chowdsp::PeakingFilter<float>,
                                   chowdsp::NotchFilter<float>,
                                   chowdsp::HighShelfFilter<float>,
                                   chowdsp::FirstOrderLPF<float>,
                                   chowdsp::SecondOrderLPF<float>,
                                   chowdsp::NthOrderFilter<float, 4, chowdsp::StateVariableFilterType::Lowpass>,
                                   chowdsp::ButterworthFilter<8, chowdsp::ButterworthFilterType::Lowpass>,
                                   chowdsp::ChebyshevIIFilter<8, chowdsp::ChebyshevFilterType::Lowpass>,
                                   chowdsp::EllipticFilter<8, chowdsp::EllipticFilterType::Lowpass>>;
    chowdsp::EQProcessor<float, Params::numBands, EQBand> eq;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PrototypeEQ)
};
