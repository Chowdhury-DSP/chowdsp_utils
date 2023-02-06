#pragma once

namespace chowdsp
{
/**
 * A normalized polynomial soft clipper. The soft clipper will
 * have a slope of 1 through the origin, and will have max/min
 * clipping values of +/- 1. Note that this clipper does not implement
 * any anti-aliasing!
 *
 * @tparam degree   The degree of the polynomial. Use a higher degree to get closer to a hard-clipper. Note that that the degree must be an odd integer.
 * @tparam T        The data type to process. Must be float/double (for now)
 */
template <int degree, typename T = float>
class SoftClipper
{
public:
    static_assert (degree % 2 == 1 && degree > 2, "Degree must be an odd integer, larger than 2!");

    SoftClipper() = default;

    /** Prepare the soft clipper to process a stream of data. */
    void prepare (const juce::dsp::ProcessSpec& spec)
    {
        exponentData = std::vector<T> (spec.maximumBlockSize, (T) 0);
    }

    /** Process an individual sample. */
    static inline T processSample (T x) noexcept
    {
        x = juce::jlimit ((T) -1, (T) 1, x * normFactor);
        x = x - Power::ipow<degree> (x) * oneOverDeg;
        return x * invNormFactor;
    }

    /** Process a block of samples. */
    void processBlock (const BufferView<T>& buffer) noexcept
    {
        const auto numSamples = buffer.getNumSamples();
        for (auto [_, channelData] : buffer_iters::channels (buffer))
        {
            juce::FloatVectorOperations::multiply (channelData.data(), normFactor, numSamples);
            juce::FloatVectorOperations::clip (channelData.data(), channelData.data(), (T) -1, (T) 1, numSamples);

            FloatVectorOperations::integerPower (exponentData.data(), channelData.data(), degree, numSamples);
            juce::FloatVectorOperations::multiply (exponentData.data(), oneOverDeg, numSamples);
            juce::FloatVectorOperations::subtract (channelData.data(), exponentData.data(), numSamples);

            juce::FloatVectorOperations::multiply (channelData.data(), invNormFactor, numSamples);
        }
    }

    /** Process a block the given processing context. */
    template <typename ProcessContext>
    void process (const ProcessContext& context) noexcept
    {
        const auto& inputBlock = context.getInputBlock();
        auto& outputBlock = context.getOutputBlock();

        const auto numSamples = (int) inputBlock.getNumSamples();
        const auto numChannels = inputBlock.getNumChannels();
        jassert (outputBlock.getNumChannels() == numChannels);

        for (size_t ch = 0; ch < numChannels; ++ch)
        {
            T* channelData;
            if (context.usesSeparateInputAndOutputBlocks())
            {
                channelData = outputBlock.getChannelPointer (ch);
                juce::FloatVectorOperations::copy (channelData, inputBlock.getChannelPointer (ch), numSamples);
            }
            else
            {
                channelData = outputBlock.getChannelPointer (ch);
            }

            if (context.isBypassed)
                continue;

            juce::FloatVectorOperations::multiply (channelData, normFactor, numSamples);
            juce::FloatVectorOperations::clip (channelData, channelData, (T) -1, (T) 1, numSamples);

            FloatVectorOperations::integerPower (exponentData.data(), channelData, degree, numSamples);
            juce::FloatVectorOperations::multiply (exponentData.data(), oneOverDeg, numSamples);
            juce::FloatVectorOperations::subtract (channelData, exponentData.data(), numSamples);

            juce::FloatVectorOperations::multiply (channelData, invNormFactor, numSamples);
        }
    }

private:
    static constexpr auto oneOverDeg = (T) 1 / (T) degree;
    static constexpr auto normFactor = T (degree - 1) / (T) degree;
    static constexpr auto invNormFactor = (T) 1 / normFactor;

    std::vector<T> exponentData;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SoftClipper)
};

} // namespace chowdsp
