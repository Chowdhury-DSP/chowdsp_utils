#pragma once

namespace chowdsp
{
/**
 * A polyphase FIR interpolation filter.
 *
 * Reference: http://www.ws.binghamton.edu/fowler/fowler%20personal%20page/EE521_files/IV-05%20Polyphase%20FIlters%20Revised.pdf
 */
template <typename T, size_t interpolationFactor, size_t numCoeffs, size_t maxChannelCount = defaultChannelCount>
class FIRPolyphaseInterpolator
{
public:
    FIRPolyphaseInterpolator() = default;

    /** Prepares the filter to process a stream of data, with the given configuration and filter coefficients. */
    template <typename ArenaType>
    void prepare (int numChannels,
                  int maxBlockSizeIn,
                  const nonstd::span<const T, numCoeffs> coeffs,
                  ArenaType& arena,
                  float extraGain = 1.0f)
    {
        std::array<T, coeffsPerFilter> oneFilterCoeffs {};

        for (size_t i = 0; i < interpolationFactor; ++i)
        {
            buffers[i] = make_temp_buffer<T> (arena, numChannels, maxBlockSizeIn);
            buffers[i].clear();
            filters[i].prepare (numChannels);

            std::fill (oneFilterCoeffs.begin(), oneFilterCoeffs.end(), T {});
            for (size_t j = 0; j < coeffsPerFilter; ++j)
            {
                const auto index = (size_t) i + j * (size_t) interpolationFactor;
                oneFilterCoeffs[j] = index >= coeffs.size() ? T {} : coeffs[index];
            }
            juce::FloatVectorOperations::multiply (oneFilterCoeffs.data(), extraGain, oneFilterCoeffs.size());
            filters[i].setCoefficients (oneFilterCoeffs.data());
        }
    }

    /**
     * Processes a block of data.
     *
     * inBlock should have a size of numSamplesIn, and outBlock should have a size of
     * numSamplesIn * interpolationFactor.
     */
    void processBlock (const T* inBlock, T* outBlock, const int numSamplesIn, const int channel = 0) noexcept
    {
        // set up sub-buffer pointers
#if ! JUCE_TEENSY
        auto* bufferPtrs = static_cast<T**> (alloca (sizeof (T*) * interpolationFactor));
#endif
        for (size_t filterIndex = 0; filterIndex < interpolationFactor; ++filterIndex)
            bufferPtrs[filterIndex] = buffers[filterIndex].getWritePointer (channel);

        // process sub-buffers
        for (size_t filterIndex = 0; filterIndex < interpolationFactor; ++filterIndex)
            filters[filterIndex].processBlock (inBlock, bufferPtrs[filterIndex], numSamplesIn, channel);

        // fill output buffer
        for (size_t filterIndex = 0; filterIndex < interpolationFactor; ++filterIndex)
            for (int n = 0; n < numSamplesIn; ++n)
                outBlock[n * (int) interpolationFactor + (int) filterIndex] = bufferPtrs[filterIndex][n];
    }

    /**
     * Processes a block of data.
     *
     * bufferOut should have a size of bufferIn.getNumSamples() * interpolationFactor.
     */
    void processBlock (const BufferView<const T>& bufferIn, const BufferView<T>& bufferOut) noexcept
    {
        jassert (bufferIn.getNumChannels() == bufferOut.getNumChannels());
        const auto numSamples = bufferIn.getNumSamples();
        jassert (numSamples == bufferOut.getNumSamples() / (int) filters.size());

        for (auto [ch, dataIn] : buffer_iters::channels (bufferIn))
            processBlock (dataIn.data(), bufferOut.getWritePointer (ch), numSamples, ch);
    }

private:
    static constexpr auto coeffsPerFilter = Math::ceiling_divide (numCoeffs, interpolationFactor);
    std::array<FIRFilter<T, coeffsPerFilter, maxChannelCount>, interpolationFactor> filters {};

    std::array<BufferView<T>, interpolationFactor> buffers {};

#if JUCE_TEENSY
    std::array<T*, interpolationFactor> bufferPtrs {};
#endif
};
} // namespace chowdsp
