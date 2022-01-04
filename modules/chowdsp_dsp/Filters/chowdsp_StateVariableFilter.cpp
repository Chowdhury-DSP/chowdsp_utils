#include "chowdsp_StateVariableFilter.h"

namespace chowdsp
{
//==============================================================================
template <typename SampleType>
StateVariableFilter<SampleType>::StateVariableFilter()
{
    update();
}

template <typename SampleType>
void StateVariableFilter<SampleType>::setCutoffFrequency (SampleType newCutoffFrequencyHz)
{
    if constexpr (std::is_floating_point<SampleType>::value)
    {
        jassert (juce::isPositiveAndBelow (newCutoffFrequencyHz, static_cast<NumericType> (sampleRate * 0.5)));
    }
    else if constexpr (SampleTypeHelpers::IsSIMDRegister<SampleType>)
    {
        for (size_t i = 0; i < SampleType::size(); ++i)
            jassert (juce::isPositiveAndBelow (newCutoffFrequencyHz.get (i), static_cast<NumericType> (sampleRate * 0.5)));
    }

    cutoffFrequency = newCutoffFrequencyHz;
    update();
}

template <typename SampleType>
void StateVariableFilter<SampleType>::setResonance (SampleType newResonance)
{
    if constexpr (std::is_floating_point<SampleType>::value)
    {
        jassert (newResonance > static_cast<NumericType> (0));
    }
    else if constexpr (SampleTypeHelpers::IsSIMDRegister<SampleType>)
    {
        for (size_t i = 0; i < SampleType::size(); ++i)
            jassert (newResonance.get (i) > static_cast<NumericType> (0));
    }

    resonance = newResonance;
    update();
}

//==============================================================================
template <typename SampleType>
void StateVariableFilter<SampleType>::prepare (const juce::dsp::ProcessSpec& spec)
{
    jassert (spec.sampleRate > 0);
    jassert (spec.numChannels > 0);

    sampleRate = spec.sampleRate;

    s1.resize (spec.numChannels);
    s2.resize (spec.numChannels);

    reset();
    update();
}

template <typename SampleType>
void StateVariableFilter<SampleType>::reset()
{
    reset (static_cast<SampleType> (0));
}

template <typename SampleType>
void StateVariableFilter<SampleType>::reset (SampleType newValue)
{
    for (auto v : { &s1, &s2 })
        std::fill (v->begin(), v->end(), newValue);
}

template <typename SampleType>
void StateVariableFilter<SampleType>::snapToZero() noexcept
{
    for (auto v : { &s1, &s2 })
        for (auto& element : *v)
            juce::dsp::util::snapToZero (element);
}

//==============================================================================
template <typename SampleType>
void StateVariableFilter<SampleType>::update()
{
    using namespace SIMDUtils;

    if constexpr (std::is_floating_point<SampleType>::value)
        g = static_cast<NumericType> (std::tan (juce::MathConstants<NumericType>::pi * cutoffFrequency / sampleRate));
    else if constexpr (SampleTypeHelpers::IsSIMDRegister<SampleType>)
        g = tanSIMD (juce::MathConstants<NumericType>::pi * cutoffFrequency / (NumericType) sampleRate);

    R2 = ((NumericType) 1.0 / resonance);
    h = ((NumericType) 1.0 / ((NumericType) 1.0 + R2 * g + g * g));

    gh = g * h;
    g2 = static_cast<NumericType> (2) * g;
    gpR2 = g + R2;
}

//==============================================================================
template class StateVariableFilter<float>;
template class StateVariableFilter<double>;
template class StateVariableFilter<juce::dsp::SIMDRegister<float>>;
template class StateVariableFilter<juce::dsp::SIMDRegister<double>>;

} // namespace chowdsp
