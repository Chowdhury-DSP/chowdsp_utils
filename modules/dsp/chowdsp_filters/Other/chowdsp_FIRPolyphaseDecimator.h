#pragma once

namespace chowdsp
{
/**
 * A polyphase FIR decimation filter.
 *
 * Reference: http://www.ws.binghamton.edu/fowler/fowler%20personal%20page/EE521_files/IV-05%20Polyphase%20FIlters%20Revised.pdf
 */
template <typename T, size_t decimationFactor, size_t numCoeffs, size_t maxChannelCount = defaultChannelCount>
class FIRPolyphaseDecimator
{
public:
    FIRPolyphaseDecimator() = default;

    /** Prepares the filter to process a stream of data, with the given configuration and filter coefficients. */
    template <typename ArenaType>
    void prepare (int numChannels,
                  int maxBlockSizeIn,
                  const nonstd::span<const T, numCoeffs> coeffs,
                  ArenaType& arena)
    {
        jassert (maxBlockSizeIn % (int) decimationFactor == 0);

        std::array<T, coeffsPerFilter> oneFilterCoeffs {};

        if constexpr (maxChannelCount == dynamicChannelCount)
            overlapState = arena::make_span<T> (arena, (size_t) numChannels * decimationFactor);
        std::fill (overlapState.begin(), overlapState.end(), T {});

        for (size_t i = 0; i < decimationFactor; ++i)
        {
            buffers[i] = make_temp_buffer<T> (arena, numChannels, maxBlockSizeIn / (int) decimationFactor);
            buffers[i].clear();
            filters[i].prepare (numChannels);

            std::fill (oneFilterCoeffs.begin(), oneFilterCoeffs.end(), T {});
            for (size_t j = 0; j < coeffsPerFilter; ++j)
            {
                const auto index = (size_t) i + j * (size_t) decimationFactor;
                oneFilterCoeffs[j] = index >= coeffs.size() ? T {} : coeffs[index];
            }
            filters[i].setCoefficients (oneFilterCoeffs.data());
        }
    }

    /**
     * Processes a block of data.
     *
     * inBlock should have a size of numSamplesIn, and outBlock should have a size of
     * numSamplesIn / decimationFactor.
     */
    void processBlock (const T* inBlock, T* outBlock, const int numSamplesIn, const int channel = 0) noexcept
    {
        jassert (numSamplesIn % (int) decimationFactor == 0);
        const auto numSamplesOut = numSamplesIn / (int) decimationFactor;

        // set up sub-buffer pointers
#if ! JUCE_TEENSY
        auto* bufferPtrs = static_cast<T**> (alloca (sizeof (T*) * decimationFactor));
#endif
        for (size_t filterIndex = 0; filterIndex < decimationFactor; ++filterIndex)
            bufferPtrs[filterIndex] = buffers[filterIndex].getWritePointer (channel);
        auto* channelOverlapState = overlapState.data() + (size_t) channel * decimationFactor;

        // fill sub-buffers
        for (int n = 0; n < numSamplesOut; ++n)
            bufferPtrs[0][n] = inBlock[n * (int) decimationFactor];

        for (size_t filterIndex = 1; filterIndex < decimationFactor; ++filterIndex)
        {
            bufferPtrs[decimationFactor - filterIndex][0] = channelOverlapState[filterIndex];
            for (int n = 0; n < numSamplesOut - 1; ++n)
                bufferPtrs[decimationFactor - filterIndex][n + 1] = inBlock[n * (int) decimationFactor + (int) filterIndex];
            channelOverlapState[filterIndex] = inBlock[(numSamplesOut - 1) * (int) decimationFactor + (int) filterIndex];
        }

        // process sub-buffers
        for (size_t filterIndex = 0; filterIndex < decimationFactor; ++filterIndex)
            filters[filterIndex].processBlock (bufferPtrs[filterIndex], numSamplesOut, channel);

        // sum the sub-buffers
        juce::FloatVectorOperations::copy (outBlock, bufferPtrs[0], numSamplesOut);
        for (size_t filterIndex = 1; filterIndex < decimationFactor; ++filterIndex)
            juce::FloatVectorOperations::add (outBlock, bufferPtrs[filterIndex], numSamplesOut);
    }

    /**
     * Processes a block of data.
     *
     * bufferOut should have a size of bufferIn.getNumSamples() / decimationFactor.
     */
    void processBlock (const BufferView<const T>& bufferIn, const BufferView<T>& bufferOut) noexcept
    {
        jassert (bufferIn.getNumChannels() == bufferOut.getNumChannels());
        const auto numSamples = bufferIn.getNumSamples();
        jassert (numSamples == bufferOut.getNumSamples() * (int) filters.size());

        for (auto [ch, dataIn] : buffer_iters::channels (bufferIn))
            processBlock (dataIn.data(), bufferOut.getWritePointer (ch), numSamples, ch);
    }

private:
    static constexpr auto coeffsPerFilter = Math::ceiling_divide (numCoeffs, decimationFactor);
    std::array<FIRFilter<T, coeffsPerFilter, maxChannelCount>, decimationFactor> filters {};

    std::array<BufferView<T>, decimationFactor> buffers {};

    using OverlapState = std::conditional_t<maxChannelCount == dynamicChannelCount,
                                            nonstd::span<T>,
                                            std::array<T, maxChannelCount * decimationFactor>>;
    OverlapState overlapState {};

#if JUCE_TEENSY
    std::array<T*, decimationFactor> bufferPtrs {};
#endif
};
} // namespace chowdsp
