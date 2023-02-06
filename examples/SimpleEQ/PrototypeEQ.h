#pragma once

#include <chowdsp_eq/chowdsp_eq.h>

using EQParams = chowdsp::EQ::StandardEQParameters<3>;
static constexpr auto DecrampedMode = chowdsp::CoefficientCalculators::CoefficientCalculationMode::Decramped;
template <typename FloatType>
using EQBand = chowdsp::EQ::EQBand<FloatType,
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
using PrototypeEQ = chowdsp::EQ::LinearPhasePrototypeEQ<float, EQParams::Params, EQParams::EQNumBands, EQBand<float>>;
