namespace chowdsp
{
template <typename T>
void Noise<T>::prepare (const juce::dsp::ProcessSpec& spec) noexcept
{
    juce::dsp::Gain<T>::prepare (spec);

    randBuffer.setSize ((int) spec.numChannels, (int) spec.maximumBlockSize);
    randBlock = juce::dsp::AudioBlock<T> (randBuffer);
    randBlock.clear();

    pink.reset (spec.numChannels);
}

template <typename T>
void Noise<T>::reset() noexcept
{
    juce::dsp::Gain<T>::reset();
}

namespace NoiseHelpers
{
    /** Returns a uniform random number in [0, 1) */
    template <typename T>
    T uniform01 (juce::Random&) noexcept;

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

    /** Generates white noise with a uniform distribution */
    template <typename T>
    struct uniformCentered
    {
        T operator() (size_t /*ch*/, juce::Random& r)
        {
            return (T) 2 * uniform01<T> (r) - (T) 1;
        }
    };

    /** Generates white noise with a normal (Gaussian) distribution */
    template <typename T>
    struct normal
    {
        T operator() (size_t /*ch*/, juce::Random& r)
        {
            // Box-Muller transform
            T radius = std::sqrt ((T) -2 * std::log ((T) 1 - uniform01<T> (r)));
            T theta = juce::MathConstants<T>::twoPi * uniform01<T> (r);
            T value = radius * std::sin (theta) / juce::MathConstants<T>::sqrt2;
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
    juce::dsp::Gain<T>::process (randContext);

    // copy input to output if needed
    if (context.usesSeparateInputAndOutputBlocks())
        outBlock.copyFrom (inBlock);

    // add random to output
    outBlock += randBlock;
}

} // namespace chowdsp