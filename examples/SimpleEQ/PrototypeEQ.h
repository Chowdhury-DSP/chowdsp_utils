#pragma once

#include <chowdsp_eq/chowdsp_eq.h>

/** "Prototype" EQ from which both the regular and linear phase EQ will be constructed. */
class PrototypeEQ
{
public:
    PrototypeEQ() = default;

    using EQParams = chowdsp::EQ::StandardEQParameters<3>;
    using FloatType = float;
    using Params = EQParams::Params;

    void setParameters (const Params& params);

    void prepare (const juce::dsp::ProcessSpec& spec);

    void reset();

    void processBlock (const chowdsp::BufferView<FloatType>& buffer);

private:
    static constexpr auto DecrampedMode = chowdsp::CoefficientCalculators::CoefficientCalculationMode::Decramped;
    using EQBand = chowdsp::EQ::EQBand<float,
                                       chowdsp::FirstOrderHPF<FloatType>,
                                       chowdsp::SecondOrderHPF<FloatType, DecrampedMode>,
                                       chowdsp::SVFHighpass<FloatType>,
                                       chowdsp::ButterworthFilter<3, chowdsp::ButterworthFilterType::Highpass, FloatType>,
                                       chowdsp::NthOrderFilter<FloatType, 4, chowdsp::StateVariableFilterType::Highpass>,
                                       chowdsp::ButterworthFilter<8, chowdsp::ButterworthFilterType::Highpass, FloatType>,
                                       chowdsp::ChebyshevIIFilter<8, chowdsp::ChebyshevFilterType::Highpass, 60, true, FloatType>,
                                       chowdsp::EllipticFilter<12, chowdsp::EllipticFilterType::Highpass, 60, chowdsp::Ratio<1, 10>, FloatType>,
                                       chowdsp::LowShelfFilter<FloatType, DecrampedMode>,
                                       chowdsp::SVFLowShelf<FloatType>,
                                       chowdsp::PeakingFilter<FloatType, DecrampedMode>,
                                       chowdsp::SVFBell<FloatType>,
                                       chowdsp::NotchFilter<FloatType, DecrampedMode>,
                                       chowdsp::SVFNotch<FloatType>,
                                       chowdsp::HighShelfFilter<FloatType, DecrampedMode>,
                                       chowdsp::SVFHighShelf<FloatType>,
                                       chowdsp::SecondOrderBPF<FloatType, DecrampedMode>,
                                       chowdsp::SVFBandpass<FloatType>,
                                       chowdsp::FirstOrderLPF<FloatType>,
                                       chowdsp::SecondOrderLPF<FloatType, DecrampedMode>,
                                       chowdsp::SVFLowpass<FloatType>,
                                       chowdsp::ButterworthFilter<3, chowdsp::ButterworthFilterType::Lowpass, FloatType>,
                                       chowdsp::NthOrderFilter<FloatType, 4, chowdsp::StateVariableFilterType::Lowpass>,
                                       chowdsp::ButterworthFilter<8, chowdsp::ButterworthFilterType::Lowpass, FloatType>,
                                       chowdsp::ChebyshevIIFilter<8, chowdsp::ChebyshevFilterType::Lowpass, 60, true, FloatType>,
                                       chowdsp::EllipticFilter<12, chowdsp::EllipticFilterType::Lowpass, 60, chowdsp::Ratio<1, 10>, FloatType>>;
    chowdsp::EQ::EQProcessor<FloatType, EQParams::EQNumBands, EQBand> eq;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PrototypeEQ)
};
