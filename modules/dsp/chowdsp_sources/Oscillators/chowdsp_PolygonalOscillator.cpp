namespace chowdsp::experimental
{
template <typename FloatType>
void PolygonalOscillator<FloatType>::setFrequency (FloatType newFrequency) noexcept
{
    freq = newFrequency;
    deltaPhase = juce::MathConstants<NumericType>::twoPi * freq / fs;
}

template <typename FloatType>
void PolygonalOscillator<FloatType>::setOrder (FloatType newOrder) noexcept
{
    order = newOrder;
    rOrder = (FloatType) 1 / order;

    CHOWDSP_USING_XSIMD_STD (cos);
    cosPiOverOrder = cos (juce::MathConstants<NumericType>::pi * rOrder);
}

template <typename FloatType>
void PolygonalOscillator<FloatType>::setTeeth (FloatType newTeeth) noexcept
{
    teeth = newTeeth;
}

template <typename FloatType>
void PolygonalOscillator<FloatType>::prepare (const juce::dsp::ProcessSpec& spec) noexcept
{
    fs = (NumericType) spec.sampleRate;
    reset();
    setFrequency (freq);
}

template <typename FloatType>
void PolygonalOscillator<FloatType>::reset() noexcept
{
    phase = {};
}

template <typename FloatType>
void PolygonalOscillator<FloatType>::reset (FloatType newPhase) noexcept
{
    phase = newPhase;
}

template <typename FloatType>
void PolygonalOscillator<FloatType>::processBlock (const BufferView<FloatType>& buffer) noexcept
{
    const auto initialPhase = phase;
    for (auto [_, x] : buffer_iters::channels (buffer))
    {
        phase = initialPhase;
        for (auto& x_n : x)
            x_n += processSample();
    }
}
} // namespace chowdsp::experimental
