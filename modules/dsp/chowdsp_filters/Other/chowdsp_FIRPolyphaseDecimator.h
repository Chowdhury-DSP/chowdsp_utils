#pragma once

namespace chowdsp
{
/**
 * A polyphase FIR decimation filter.
 *
 * Reference: http://www.ws.binghamton.edu/fowler/fowler%20personal%20page/EE521_files/IV-05%20Polyphase%20FIlters%20Revised.pdf
 */
template <typename T>
class FIRPolyphaseDecimator
{
public:
    FIRPolyphaseDecimator() = default;

    /** Prepares the filter to process a stream of data, with the given configuration and filter coefficients. */
    void prepare (int decimationFactor, int numChannels, int maxBlockSizeIn, const nonstd::span<const T> coeffs)
    {
        jassert (maxBlockSizeIn % decimationFactor == 0);
        const auto numCoeffs = coeffs.size();
        const auto coeffsPerFilter = Math::ceiling_divide (numCoeffs, (size_t) decimationFactor);

        std::vector<float> oneFilterCoeffs (coeffsPerFilter);

#if JUCE_TEENSY
        bufferPtrs.resize ((size_t) decimationFactor);
#endif

        buffers.clear();
        buffers.reserve ((size_t) decimationFactor);
        overlapState = std::vector<T> ((size_t) numChannels * (size_t) decimationFactor, T {});
        filters.clear();
        filters.reserve ((size_t) decimationFactor);
        for (int i = 0; i < decimationFactor; ++i)
        {
            buffers.emplace_back (numChannels, maxBlockSizeIn / decimationFactor);

            auto& filter = filters.emplace_back ((int) coeffsPerFilter);
            filter.prepare (numChannels);

            std::fill (oneFilterCoeffs.begin(), oneFilterCoeffs.end(), T {});
            for (size_t j = 0; j < coeffsPerFilter; ++j)
            {
                const auto index = (size_t) i + j * (size_t) decimationFactor;
                oneFilterCoeffs[j] = index >= coeffs.size() ? T {} : coeffs[index];
            }
            filter.setCoefficients (oneFilterCoeffs.data());
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
        const auto decimationFactor = (int) filters.size();
        jassert (numSamplesIn % decimationFactor == 0);
        const auto numSamplesOut = numSamplesIn / decimationFactor;

        // set up sub-buffer pointers
#if ! JUCE_TEENSY
        auto* bufferPtrs = static_cast<T**> (alloca (sizeof (T*) * (size_t) decimationFactor));
#endif
        for (size_t filterIndex = 0; filterIndex < (size_t) decimationFactor; ++filterIndex)
            bufferPtrs[filterIndex] = buffers[filterIndex].getWritePointer (channel);
        auto* channelOverlapState = overlapState.data() + (size_t) channel * (size_t) decimationFactor;

        // fill sub-buffers
        for (int n = 0; n < numSamplesOut; ++n)
            bufferPtrs[0][n] = inBlock[n * decimationFactor];

        for (size_t filterIndex = 1; filterIndex < (size_t) decimationFactor; ++filterIndex)
        {
            bufferPtrs[(size_t) decimationFactor - filterIndex][0] = channelOverlapState[filterIndex];
            for (int n = 0; n < numSamplesOut - 1; ++n)
                bufferPtrs[(size_t) decimationFactor - filterIndex][n + 1] = inBlock[n * decimationFactor + (int) filterIndex];
            channelOverlapState[filterIndex] = inBlock[(numSamplesOut - 1) * decimationFactor + (int) filterIndex];
        }

        // process sub-buffers
        for (size_t filterIndex = 0; filterIndex < (size_t) decimationFactor; ++filterIndex)
            filters[filterIndex].processBlock (bufferPtrs[filterIndex], numSamplesOut, channel);

        // sum the sub-buffers
        juce::FloatVectorOperations::copy (outBlock, bufferPtrs[0], numSamplesOut);
        for (size_t filterIndex = 1; filterIndex < (size_t) decimationFactor; ++filterIndex)
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
    std::vector<FIRFilter<T>> filters {};

    std::vector<Buffer<T>> buffers {};
    std::vector<T> overlapState {};

#if JUCE_TEENSY
    std::vector<T*> bufferPtrs {};
#endif
};
} // namespace chowdsp
