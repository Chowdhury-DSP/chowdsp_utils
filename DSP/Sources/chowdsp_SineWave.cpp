namespace chowdsp
{
template <typename T>
void SineWave<T>::setFrequency (T newFrequency) noexcept
{
    freq = newFrequency;
    eps = 2 * std::sin (juce::MathConstants<T>::pi * newFrequency / fs);
}

template <typename T>
void SineWave<T>::prepare (const juce::dsp::ProcessSpec& spec) noexcept
{
    fs = static_cast<T> (spec.sampleRate);
    reset();
}

template <typename T>
void SineWave<T>::reset() noexcept
{
    // reset state to be "in phase"
    x1 = -1.0f;
    x2 = 0.0f;
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
