namespace chowdsp
{
template <typename T>
void TriangleWave<T>::setFrequency (T newFrequency) noexcept
{
    freq = newFrequency;
    deltaPhase = (T) 2 * freq / fs;

    // scale by zero if freq == 0, to avoid divide by zero issue
    waveformPreservingScale = fs / ((T) 2 * freq);
    waveformPreservingScale = SIMDUtils::select (freq == (T) 0, (T) 0, waveformPreservingScale);
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

    T z_temp = z;
    T phi_temp = phi;

    for (int ch = 0; ch < numChannels; ++ch)
    {
        z = z_temp;
        phi = phi_temp;

        ScopedValue _z { z };
        ScopedValue _phi { phi };

        auto* data = buffer.getWritePointer (ch);
        for (int i = 0; i < numSamples; ++i)
            data[i] += processSampleInternal (_z.get(), _phi.get());
    }
}
} // namespace chowdsp
