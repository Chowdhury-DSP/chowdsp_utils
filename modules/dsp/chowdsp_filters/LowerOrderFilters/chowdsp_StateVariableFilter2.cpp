#include "chowdsp_StateVariableFilter2.h"

namespace chowdsp
{
template <typename SampleType, StateVariableFilter2Type type>
StateVariableFilter2<SampleType, type>::StateVariableFilter2()
{
    setCutoffFrequency (static_cast<NumericType> (1000.0));
    setQValue (static_cast<NumericType> (1.0 / juce::MathConstants<double>::sqrt2));
}

template <typename SampleType, StateVariableFilter2Type type>
void StateVariableFilter2<SampleType, type>::setCutoffFrequency (SampleType newCutoffFrequencyHz)
{
    jassert (SIMDUtils::all (newCutoffFrequencyHz >= static_cast<NumericType> (0)));
    jassert (SIMDUtils::all (newCutoffFrequencyHz < static_cast<NumericType> (sampleRate * 0.5)));

    cutoffFrequency = newCutoffFrequencyHz;
    const auto w = juce::MathConstants<NumericType>::pi * cutoffFrequency / (NumericType) sampleRate;

    CHOWDSP_USING_XSIMD_STD (tan)
    g0 = tan (w);

    update();
}

template <typename SampleType, StateVariableFilter2Type type>
void StateVariableFilter2<SampleType, type>::setQValue (SampleType newResonance)
{
    SIMDUtils::all (newResonance > static_cast<NumericType> (0));

    resonance = newResonance;
    k0 = (NumericType) 1.0 / resonance;

    update();
}

template <typename SampleType, StateVariableFilter2Type type>
void StateVariableFilter2<SampleType, type>::prepare (const juce::dsp::ProcessSpec& spec)
{
    jassert (spec.sampleRate > 0);
    jassert (spec.numChannels > 0);

    sampleRate = spec.sampleRate;

    ic1eq.resize (spec.numChannels);
    ic2eq.resize (spec.numChannels);

    reset();
    update();
}

template <typename SampleType, StateVariableFilter2Type type>
void StateVariableFilter2<SampleType, type>::reset()
{
    for (auto v : { &ic1eq, &ic2eq })
        std::fill (v->begin(), v->end(), static_cast<SampleType> (0));
}

template <typename SampleType, StateVariableFilter2Type type>
void StateVariableFilter2<SampleType, type>::snapToZero() noexcept
{
    for (auto v : { &ic1eq, &ic2eq })
        for (auto& element : *v)
            juce::dsp::util::snapToZero (element);
}

template <typename SampleType, StateVariableFilter2Type type>
void StateVariableFilter2<SampleType, type>::update()
{
    //    const auto g = g0;
    //    k = k0;

    const auto gk = g0 + k0;
    a1 = (NumericType) 1.0 / ((NumericType) 1.0 + g0 * gk);
    a2 = g0 * a1;
    a3 = g0 * a2;
    ak = gk * a1;
}
} // namespace chowdsp
