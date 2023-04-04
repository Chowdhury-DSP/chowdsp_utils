namespace chowdsp
{
template <typename T>
void Noise<T>::prepare (const juce::dsp::ProcessSpec& spec) noexcept
{
    juce::dsp::Gain<NumericType>::prepare (spec);

    randBlock = AudioBlock<T> (randBlockData, spec.numChannels, spec.maximumBlockSize);
    randBlock.clear();

    gainBlock = AudioBlock<T> (gainBlockData, spec.numChannels, spec.maximumBlockSize);
    gainBlock.clear();

    pink.reset (spec.numChannels);
}

template <typename T>
void Noise<T>::reset() noexcept
{
    juce::dsp::Gain<NumericType>::reset();
}

#ifndef DOXYGEN
namespace NoiseHelpers
{
    using namespace SIMDUtils;

    /** Returns a uniform random number in [0, 1) */
    template <>
    inline double uniform01 (juce::Random& r) noexcept
    {
        return r.nextDouble();
    }

    /** Returns a uniform random number in [0, 1) */
    template <>
    inline float uniform01 (juce::Random& r) noexcept
    {
        return r.nextFloat();
    }

    /** Returns a uniform random number in [0, 1) */
    template <>
    inline xsimd::batch<double> uniform01 (juce::Random& r) noexcept
    {
        double sample alignas (xsimd::default_arch::alignment())[xsimd::batch<double>::size];
        for (auto& x : sample)
            x = r.nextDouble();

        return xsimd::load_aligned (sample);
    }

    /** Returns a uniform random number in [0, 1) */
    template <>
    inline xsimd::batch<float> uniform01 (juce::Random& r) noexcept
    {
        float sample alignas (xsimd::default_arch::alignment())[xsimd::batch<float>::size];
        for (auto& x : sample)
            x = r.nextFloat();

        return xsimd::load_aligned (sample);
    }

    /** Generates white noise with a uniform distribution */
    template <typename T>
    struct uniformCentered
    {
        inline T operator() (size_t /*ch*/, juce::Random& r) const noexcept
        {
            return (T) 2 * uniform01<T> (r) - (T) 1;
        }
    };

    /** Generates white noise with a normal (Gaussian) distribution */
    template <typename T>
    struct normal
    {
        using NumericType = SampleTypeHelpers::NumericType<T>;

        inline T operator() (size_t /*ch*/, juce::Random& r) const noexcept
        {
            CHOWDSP_USING_XSIMD_STD (sqrt);
            CHOWDSP_USING_XSIMD_STD (log);
            CHOWDSP_USING_XSIMD_STD (sin);

            // Box-Muller transform
            T radius = sqrt ((T) -2 * log ((T) 1 - uniform01<T> (r)));
            T theta = juce::MathConstants<NumericType>::twoPi * uniform01<T> (r);
            T value = radius * sin (theta) / juce::MathConstants<NumericType>::sqrt2;
            return value;
        }
    };

    /** Process audio context with some random functor */
    template <typename T, typename ProcessContext, typename F>
    void processRandom (const ProcessContext& context, juce::Random& r, F randFunc) noexcept
    {
        auto&& outBlock = context.getOutputBlock();

        auto len = outBlock.getNumSamples();
        auto numChannels = outBlock.getNumChannels();

        for (size_t ch = 0; ch < numChannels; ++ch)
        {
            auto* dst = outBlock.getChannelPointer (ch);

            for (size_t i = 0; i < len; ++i)
                dst[i] = randFunc (ch, r);
        }
    }

} // namespace NoiseHelpers
#endif // DOXYGEN

template <typename T>
template <typename ProcessContext>
void Noise<T>::process (const ProcessContext& context) noexcept
{
    // bypass
    if (context.isBypassed)
        return;

    auto&& outBlock = context.getOutputBlock();
    auto&& inBlock = context.getInputBlock();
    auto len = outBlock.getNumSamples();

    auto randSubBlock = randBlock.getSubBlock (0, len);
    ProcessContextReplacing<T> randContext (randSubBlock);

    // generate random block
    if (type == Uniform)
        NoiseHelpers::processRandom<T> (randContext, rand, NoiseHelpers::uniformCentered<T>());
    else if (type == Normal)
        NoiseHelpers::processRandom<T> (randContext, rand, NoiseHelpers::normal<T>());
    else if (type == Pink)
        NoiseHelpers::processRandom<T> (randContext, rand, pink);

    // apply gain to random block
    applyGain (randSubBlock);

    // copy input to output if needed
    if (context.usesSeparateInputAndOutputBlocks())
        outBlock.copyFrom (inBlock);

    // add random to output
    outBlock += randBlock;
}

template <typename T>
void Noise<T>::processBlock (const BufferView<T>& buffer) noexcept
{
    auto&& block = buffer.toAudioBlock();
    process (juce::dsp::ProcessContextReplacing<float> { block });
}
} // namespace chowdsp