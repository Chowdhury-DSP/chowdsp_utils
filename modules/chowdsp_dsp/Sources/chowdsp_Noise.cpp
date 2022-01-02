namespace chowdsp
{
template <typename T>
void Noise<T>::prepare (const juce::dsp::ProcessSpec& spec) noexcept
{
    juce::dsp::Gain<NumericType>::prepare (spec);

    randBlock = juce::dsp::AudioBlock<T> (randBlockData, spec.numChannels, spec.maximumBlockSize);
    randBlock.clear();

    gainBlock = juce::dsp::AudioBlock<T> (gainBlockData, spec.numChannels, spec.maximumBlockSize);
    gainBlock.clear();

    pink.reset (spec.numChannels);
}

template <typename T>
void Noise<T>::reset() noexcept
{
    juce::dsp::Gain<T>::reset();
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
    inline vec2 uniform01 (juce::Random& r) noexcept
    {
        double sample alignas (16)[vec2::size()];
        for (size_t i = 0; i < vec2::size(); ++i)
            sample[i] = r.nextDouble();

        return vec2::fromRawArray (sample);
    }

    /** Returns a uniform random number in [0, 1) */
    template <>
    inline vec4 uniform01 (juce::Random& r) noexcept
    {
        float sample alignas (16)[vec4::size()];
        for (size_t i = 0; i < vec4::size(); ++i)
            sample[i] = r.nextFloat();

        return vec4::fromRawArray (sample);
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
        using NumericType = typename SampleTypeHelpers::ElementType<T>::Type;

        template <typename C = T>
        inline typename std::enable_if<std::is_floating_point<C>::value, C>::type
            operator() (size_t /*ch*/, juce::Random& r) const noexcept
        {
            // Box-Muller transform
            T radius = std::sqrt ((T) -2 * std::log ((T) 1 - uniform01<T> (r)));
            T theta = juce::MathConstants<NumericType>::twoPi * uniform01<T> (r);
            T value = radius * std::sin (theta) / juce::MathConstants<NumericType>::sqrt2;
            return value;
        }

        template <typename C = T>
        inline typename std::enable_if<! std::is_floating_point<C>::value, C>::type
            operator() (size_t /*ch*/, juce::Random& r) const noexcept
        {
            // Box-Muller transform
            T radius = sqrtSIMD ((T) -2 * logSIMD ((T) 1 - uniform01<T> (r)));
            T theta = uniform01<T> (r) * juce::MathConstants<NumericType>::twoPi;
            T value = radius * sinSIMD (theta) / juce::MathConstants<NumericType>::sqrt2;
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
    juce::dsp::ProcessContextReplacing<T> randContext (randSubBlock);

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
        AudioBlockHelpers::copyBlocks (outBlock, inBlock);

    // add random to output
    outBlock += randBlock;
}

} // namespace chowdsp