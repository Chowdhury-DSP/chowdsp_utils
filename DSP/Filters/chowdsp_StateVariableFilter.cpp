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
    jassert (juce::isPositiveAndBelow (newCutoffFrequencyHz, static_cast<SampleType> (sampleRate * 0.5)));

    cutoffFrequency = newCutoffFrequencyHz;
    update();
}

template <typename SampleType>
void StateVariableFilter<SampleType>::setResonance (SampleType newResonance)
{
    jassert (newResonance > static_cast<SampleType> (0));

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
    g = static_cast<SampleType> (std::tan (juce::MathConstants<double>::pi * cutoffFrequency / sampleRate));
    R2 = static_cast<SampleType> (1.0 / resonance);
    h = static_cast<SampleType> (1.0 / (1.0 + R2 * g + g * g));

    gh = g * h;
    g2 = static_cast<SampleType> (2) * g;
    gpR2 = g + R2;
}

//==============================================================================
template class StateVariableFilter<float>;
template class StateVariableFilter<double>;

} // namespace chowdsp
