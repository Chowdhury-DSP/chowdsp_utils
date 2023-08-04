namespace chowdsp
{
template <typename T>
void SawtoothWave<T>::setFrequency (T newFrequency) noexcept
{
    freq = newFrequency;
    deltaPhase = (T) 2 * freq / fs;

    // scale by zero if freq == 0, to avoid divide by zero issue
    JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wfloat-equal")
    waveformPreservingScale = fs / ((T) 4 * freq);
    waveformPreservingScale = SIMDUtils::select (freq == (T) 0, (T) 0, waveformPreservingScale);
    JUCE_END_IGNORE_WARNINGS_GCC_LIKE
}

template <typename T>
void SawtoothWave<T>::prepare (const juce::dsp::ProcessSpec& spec) noexcept
{
    using NumericType = SampleTypeHelpers::NumericType<T>;
    fs = (T) (NumericType) spec.sampleRate;
    reset();
}

template <typename T>
void SawtoothWave<T>::reset (T phase) noexcept
{
    // make sure initial phase is in range
    phi = phase - deltaPhase;
    updatePhase();

    z = phi * phi;
}

template <typename T>
void SawtoothWave<T>::processBlock (const BufferView<T>& buffer) noexcept
{
    T z_temp = z;
    T phi_temp = phi;

    for (auto [_, data] : buffer_iters::channels (buffer))
    {
        z = z_temp;
        phi = phi_temp;

        for (auto& x_n : data)
            x_n += processSample();
    }
}

template <typename T>
template <typename ProcessContext>
void SawtoothWave<T>::process (const ProcessContext& context) noexcept
{
    auto&& outBlock = context.getOutputBlock();
    auto&& inBlock = context.getInputBlock();

    auto len = outBlock.getNumSamples();
    auto numChannels = outBlock.getNumChannels();
    auto inputChannels = inBlock.getNumChannels();

    if (context.isBypassed)
    {
        if (context.usesSeparateInputAndOutputBlocks())
            context.getOutputBlock().clear();

        for (size_t i = 0; i < len; ++i)
            processSample();

        return;
    }

    T z_temp = z;
    T phi_temp = phi;
    size_t ch;

    for (ch = 0; ch < juce::jmin (numChannels, inputChannels); ++ch)
    {
        z = z_temp;
        phi = phi_temp;

        auto* dst = outBlock.getChannelPointer (ch);
        auto* src = inBlock.getChannelPointer (ch);

        if (context.usesSeparateInputAndOutputBlocks())
            juce::FloatVectorOperations::copy (dst, src, (int) len);

        for (size_t i = 0; i < len; ++i)
            dst[i] += processSample();
    }

    for (; ch < numChannels; ++ch)
    {
        z = z_temp;
        phi = phi_temp;
        auto* dst = outBlock.getChannelPointer (ch);

        for (size_t i = 0; i < len; ++i)
            dst[i] = processSample();
    }
}
} // namespace chowdsp
