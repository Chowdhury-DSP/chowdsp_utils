namespace chowdsp
{
template <typename T>
void SineWave<T>::setFrequency (T newFrequency) noexcept
{
    CHOWDSP_USING_XSIMD_STD (sin);
    freq = newFrequency;
    eps = 2 * sin (juce::MathConstants<NumericType>::pi * newFrequency / fs);
}

template <typename T>
void SineWave<T>::prepare (const juce::dsp::ProcessSpec& spec) noexcept
{
    fs = static_cast<T> ((NumericType) spec.sampleRate);
    reset();
}

template <typename T>
void SineWave<T>::reset() noexcept
{
    // reset state to be "in phase"
    x1 = (T) -1;
    x2 = (T) 0;
}

template <typename T>
void SineWave<T>::reset (T phase) noexcept
{
    CHOWDSP_USING_XSIMD_STD (sin);
    CHOWDSP_USING_XSIMD_STD (cos);
    x1 = sin (phase);
    x2 = cos (phase);
}

template <typename T>
void SineWave<T>::processBlock (const BufferView<T>& buffer) noexcept
{
    T x1_temp = x1;
    T x2_temp = x2;

    for (auto [_, data] : buffer_iters::channels (buffer))
    {
        x1_temp = x1;
        x2_temp = x2;

        for (auto& x_n : data)
        {
            x_n += x2_temp;
            x1_temp += eps * x2_temp;
            x2_temp -= eps * x1_temp;
        }
    }

    x1 = x1_temp;
    x2 = x2_temp;
}

template <typename T>
template <typename ProcessContext>
void SineWave<T>::process (const ProcessContext& context) noexcept
{
    auto&& outBlock = context.getOutputBlock();
    auto&& inBlock = context.getInputBlock();

    auto len = outBlock.getNumSamples();
    auto numChannels = outBlock.getNumChannels();
    auto inputChannels = inBlock.getNumChannels();

    if (context.isBypassed)
    {
        context.getOutputBlock().clear();

        for (size_t i = 0; i < len; ++i)
        {
            x1 += eps * x2;
            x2 -= eps * x1;
        }
    }
    else
    {
        T x1_temp = x1;
        T x2_temp = x2;
        size_t ch;

        if (context.usesSeparateInputAndOutputBlocks())
        {
            for (ch = 0; ch < juce::jmin (numChannels, inputChannels); ++ch)
            {
                x1_temp = x1;
                x2_temp = x2;
                auto* dst = outBlock.getChannelPointer (ch);
                auto* src = inBlock.getChannelPointer (ch);

                for (size_t i = 0; i < len; ++i)
                {
                    dst[i] = src[i] + x2_temp;
                    x1_temp += eps * x2_temp;
                    x2_temp -= eps * x1_temp;
                }
            }
        }
        else
        {
            for (ch = 0; ch < juce::jmin (numChannels, inputChannels); ++ch)
            {
                x1_temp = x1;
                x2_temp = x2;
                auto* dst = outBlock.getChannelPointer (ch);

                for (size_t i = 0; i < len; ++i)
                {
                    dst[i] += x2_temp;
                    x1_temp += eps * x2_temp;
                    x2_temp -= eps * x1_temp;
                }
            }
        }

        for (; ch < numChannels; ++ch)
        {
            x1_temp = x1;
            x2_temp = x2;
            auto* dst = outBlock.getChannelPointer (ch);

            for (size_t i = 0; i < len; ++i)
            {
                dst[i] = x2_temp;
                x1_temp += eps * x2_temp;
                x2_temp -= eps * x1_temp;
            }
        }

        x1 = x1_temp;
        x2 = x2_temp;
    }
}

} // namespace chowdsp
