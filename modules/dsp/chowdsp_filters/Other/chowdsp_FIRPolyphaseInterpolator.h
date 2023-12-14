#pragma once

namespace chowdsp
{
/**
 * A polyphase FIR interpolation filter.
 *
 * Reference: http://www.ws.binghamton.edu/fowler/fowler%20personal%20page/EE521_files/IV-05%20Polyphase%20FIlters%20Revised.pdf
 */
template <typename T>
class FIRPolyphaseInterpolator
{
public:
    FIRPolyphaseInterpolator() = default;

    /** Prepares the filter to process a stream of data, with the given configuration and filter coefficients. */
    void prepare (int interpolationFactor, int numChannels, int maxBlockSizeIn, const nonstd::span<const T> coeffs)
    {
        const auto numCoeffs = coeffs.size();
        const auto coeffsPerFilter = Math::ceiling_divide (numCoeffs, (size_t) interpolationFactor);

        std::vector<float> oneFilterCoeffs (coeffsPerFilter);

#if JUCE_TEENSY
        bufferPtrs.resize ((size_t) interpolationFactor);
#endif

        buffers.clear();
        buffers.reserve ((size_t) interpolationFactor);
        filters.clear();
        filters.reserve ((size_t) interpolationFactor);
        for (int i = 0; i < interpolationFactor; ++i)
        {
            buffers.emplace_back (numChannels, maxBlockSizeIn);

            auto& filter = filters.emplace_back ((int) coeffsPerFilter);
            filter.prepare (numChannels);

            std::fill (oneFilterCoeffs.begin(), oneFilterCoeffs.end(), T {});
            for (size_t j = 0; j < coeffsPerFilter; ++j)
            {
                const auto index = (size_t) i + j * (size_t) interpolationFactor;
                oneFilterCoeffs[j] = index >= coeffs.size() ? T {} : coeffs[index];
            }
            filter.setCoefficients (oneFilterCoeffs.data());
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
        const auto interpolationFactor = (int) filters.size();

        // set up sub-buffer pointers
#if ! JUCE_TEENSY
        auto* bufferPtrs = static_cast<T**> (alloca (sizeof (T*) * (size_t) interpolationFactor));
#endif
        for (size_t filterIndex = 0; filterIndex < (size_t) interpolationFactor; ++filterIndex)
            bufferPtrs[filterIndex] = buffers[filterIndex].getWritePointer (channel);

        // process sub-buffers
        for (size_t filterIndex = 0; filterIndex < (size_t) interpolationFactor; ++filterIndex)
            filters[filterIndex].processBlock (inBlock, bufferPtrs[filterIndex], numSamplesIn, channel);

        // fill output buffer
        for (size_t filterIndex = 0; filterIndex < (size_t) interpolationFactor; ++filterIndex)
            for (int n = 0; n < numSamplesIn; ++n)
                outBlock[n * interpolationFactor + (int) filterIndex] = bufferPtrs[filterIndex][n];
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
    std::vector<FIRFilter<T>> filters {};

    std::vector<Buffer<T>> buffers {};

#if JUCE_TEENSY
    std::vector<T*> bufferPtrs {};
#endif
};
} // namespace chowdsp
