#include "chowdsp_StateVariableFilter.h"

namespace chowdsp
{
template <typename SampleType, StateVariableFilterType type>
StateVariableFilter<SampleType, type>::StateVariableFilter()
{
    setCutoffFrequency (static_cast<NumericType> (1000.0));
    setQValue (static_cast<NumericType> (1.0 / juce::MathConstants<double>::sqrt2));
    setGain (static_cast<NumericType> (1.0));
}

template <typename SampleType, StateVariableFilterType type>
template <bool shouldUpdate>
void StateVariableFilter<SampleType, type>::setCutoffFrequency (SampleType newCutoffFrequencyHz)
{
    jassert (SIMDUtils::all (newCutoffFrequencyHz >= static_cast<NumericType> (0)));
    jassert (SIMDUtils::all (newCutoffFrequencyHz < static_cast<NumericType> (sampleRate * 0.5)));

    cutoffFrequency = newCutoffFrequencyHz;
    const auto w = juce::MathConstants<NumericType>::pi * cutoffFrequency / (NumericType) sampleRate;

    CHOWDSP_USING_XSIMD_STD (tan)
    g0 = tan (w);

    if constexpr (shouldUpdate)
        update();
}

template <typename SampleType, StateVariableFilterType type>
template <bool shouldUpdate>
void StateVariableFilter<SampleType, type>::setQValue (SampleType newResonance)
{
    jassert (SIMDUtils::all (newResonance > static_cast<NumericType> (0)));

    resonance = newResonance;
    k0 = (NumericType) 1.0 / resonance;
    k0A = k0 * A;

    if constexpr (shouldUpdate)
        update();
}

template <typename SampleType, StateVariableFilterType type>
template <bool shouldUpdate>
void StateVariableFilter<SampleType, type>::setGain (SampleType newGainLinear)
{
    jassert (SIMDUtils::all (newGainLinear > static_cast<NumericType> (0)));

    gain = newGainLinear;

    CHOWDSP_USING_XSIMD_STD (sqrt)
    A = sqrt (gain);
    sqrtA = sqrt (A);
    Asq = A * A;
    k0A = k0 * A;

    if constexpr (shouldUpdate)
        update();
}

template <typename SampleType, StateVariableFilterType type>
template <bool shouldUpdate>
void StateVariableFilter<SampleType, type>::setGainDecibels (SampleType newGainDecibels)
{
    setGain<shouldUpdate> (SIMDUtils::decibelsToGain (newGainDecibels));
}

template <typename SampleType, StateVariableFilterType type>
void StateVariableFilter<SampleType, type>::prepare (const juce::dsp::ProcessSpec& spec)
{
    jassert (spec.sampleRate > 0);
    jassert (spec.numChannels > 0);

    sampleRate = spec.sampleRate;

    ic1eq.resize (spec.numChannels);
    ic2eq.resize (spec.numChannels);

    reset();

    setCutoffFrequency (cutoffFrequency);
}

template <typename SampleType, StateVariableFilterType type>
void StateVariableFilter<SampleType, type>::reset()
{
    for (auto v : { &ic1eq, &ic2eq })
        std::fill (v->begin(), v->end(), static_cast<SampleType> (0));
}

template <typename SampleType, StateVariableFilterType type>
void StateVariableFilter<SampleType, type>::snapToZero() noexcept
{
#if JUCE_SNAP_TO_ZERO
    for (auto v : { &ic1eq, &ic2eq })
        for (auto& element : *v)
            juce::dsp::util::snapToZero (element);
#endif
}

template <typename SampleType, StateVariableFilterType type>
void StateVariableFilter<SampleType, type>::update()
{
    SampleType g, k;
    if constexpr (type == FilterType::Bell)
    {
        g = g0;
        k = k0 / A;
    }
    else if constexpr (type == FilterType::LowShelf)
    {
        g = g0 / sqrtA;
        k = k0;
    }
    else if constexpr (type == FilterType::HighShelf)
    {
        g = g0 * sqrtA;
        k = k0;
    }
    else
    {
        g = g0;
        k = k0;
    }

    const auto gk = g + k;
    a1 = (NumericType) 1.0 / ((NumericType) 1.0 + g * gk);
    a2 = g * a1;
    a3 = g * a2;
    ak = gk * a1;
}
} // namespace chowdsp
