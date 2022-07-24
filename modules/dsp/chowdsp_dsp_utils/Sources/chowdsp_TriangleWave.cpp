namespace chowdsp
{
template <typename T>
void TriangleWave<T>::setFrequency (T newFrequency) noexcept
{
    using namespace SIMDUtils;

    freq = newFrequency;
    deltaPhase = (T) 2 * freq / fs;

    // scale by zero if freq == 0, to avoid divide by zero issue
    waveformPreservingScale = fs / ((T) 2 * freq);
    waveformPreservingScale = select (freq == (T) 0, (T) 0, waveformPreservingScale);
}

template <typename T>
void TriangleWave<T>::prepare (const juce::dsp::ProcessSpec& spec) noexcept
{
    using NumericType = SampleTypeHelpers::NumericType<T>;
    fs = (T) (NumericType) spec.sampleRate;
    reset();
}

template <typename T>
void TriangleWave<T>::reset (T phase) noexcept
{
    // make sure initial phase is in range
    phi = phase - deltaPhase;
    updatePhase (phi, deltaPhase);

    CHOWDSP_USING_XSIMD_STD (abs);
    z = phi * abs (phi) - phi;
}

template <typename T>
void TriangleWave<T>::processBlock (const BufferView<T>& buffer) noexcept
{
    const auto numChannels = buffer.getNumChannels();
    const auto numSamples = buffer.getNumSamples();

    {
        auto* data = buffer.getWritePointer (0);

        ScopedValue _z { z };
        ScopedValue _phi { phi };

        for (int i = 0; i < numSamples; ++i)
            data[i] += processSampleInternal (_z.get(), _phi.get());
    }

    for (int ch = 1; ch < numChannels; ++ch)
        BufferMath::copyBufferChannels (buffer, buffer, 0, ch);
}
} // namespace chowdsp
