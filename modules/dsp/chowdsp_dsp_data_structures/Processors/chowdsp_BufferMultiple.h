#pragma once

namespace chowdsp
{
/**
 * BufferMultiple can be used to process buffers that need to be a multiple of some
 * number of samples.
 *
 * BufferMultiple requires less latency than RebufferedProcessor, but RebufferedProcessor
 * should still be preferred when an exact buffer length is required.
 *
 * Here's an example where we need to process buffers that are a multiple of 3 samples,
 * for example if we wanted to down-sample by 3x:
 * @code
 * // prepare
 * BufferMultiple<float> bufferMultiple {};
 * const auto m3BufferSize = bufferMultiple.prepare (spec, 3);
 * arenaBytesNeeded += m3BufferSize * numChannels * sizeof (float);
 * nextProcessor.prepare ({ sampleRate, m3BufferSize, numChannels });
 * ...
 * arena.reset (arenaBytesNeeded);
 *
 * // use
 * auto m3Buffer = bufferMultiple.processBufferIn (arena, buffer);
 * nextProcessor.process (m3Buffer);
 * bufferMultiple.processBufferOut (m3Buffer, buffer);
 * @endcode
 */
template <typename T>
class BufferMultiple
{
public:
    BufferMultiple() = default;

    /**
     * Prepares the processor for a given multiple.
     *
     * This method returns the maximum number of samples
     * that may be allocated by the arena allocator used
     * by processBufferIn(). Note that this number may not
     * be a multiple of the requested multiple do to buffer
     * padding restrictions.
     */
    int prepare (const juce::dsp::ProcessSpec& spec, int multiple)
    {
        jassert (multiple > 1 && multiple <= static_cast<int> (maxMultiple));
        M = multiple;
        numChannels = static_cast<int> (spec.numChannels);

        reset();

        return Math::round_to_next_multiple (Math::round_to_next_multiple (static_cast<int> (spec.maximumBlockSize), M),
                                             static_cast<int> (SIMDUtils::defaultSIMDAlignment / sizeof (T)));
    }

    /** Resets the processor state. */
    void reset() noexcept
    {
        leftoverDataIn.fill (T {});
        leftoverDataOut.fill (T {});
        numSamplesLeftoverIn = M - 1;
        numSamplesLeftoverOut = 1;
    }

    /** Returns the latency of the "multiple" buffer. */
    [[nodiscard]] int getMultipleBufferLatency() const noexcept
    {
        return M - 1;
    }

    /** Returns the "round trip" latency for this processor. */
    [[nodiscard]] int getRoundTripLatency() const noexcept
    {
        return M;
    }

    /** Returns a buffer that is the requested multiple number of samples. */
    BufferView<T> processBufferIn (ArenaAllocatorView arena, const BufferView<const T>& input) noexcept
    {
        const auto numSamplesIn = input.getNumSamples();
        const auto numMultiplesOut = (numSamplesIn + numSamplesLeftoverIn - 1) / M;
        const auto numSamplesOut = M * numMultiplesOut;
        const auto newNumLeftoverSamples = numSamplesIn + numSamplesLeftoverIn - numSamplesOut;
        jassert (newNumLeftoverSamples <= M);

        auto leftoverSamplesToUse = std::min (numSamplesLeftoverIn, numSamplesOut);

        const auto bufferOut = make_temp_buffer<T> (arena, input.getNumChannels(), numSamplesOut);

        for (auto [ch, outData] : buffer_iters::channels (bufferOut))
        {
            const auto inputs = input.getReadSpan (ch);
            const auto leftovers = getLeftoversIn (ch);

            std::copy (leftovers.begin(), leftovers.begin() + leftoverSamplesToUse, outData.begin());
            std::copy (inputs.begin(), inputs.begin() + numSamplesOut - leftoverSamplesToUse, outData.begin() + leftoverSamplesToUse);

            if (leftoverSamplesToUse < numSamplesLeftoverIn)
            {
                std::copy (leftovers.begin() + leftoverSamplesToUse, leftovers.end(), leftovers.begin());
            }

            std::copy (inputs.begin() + numSamplesOut - leftoverSamplesToUse,
                       inputs.end(),
                       leftovers.begin() + numSamplesLeftoverIn - leftoverSamplesToUse);
        }
        numSamplesLeftoverIn = newNumLeftoverSamples;

        return bufferOut;
    }

    /**
     * Copies the multiple buffer into some output buffer.
     *
     * The output buffer is expected to be the same size as the
     * most recent buffer provided to processBufferIn.
     */
    void processBufferOut (const BufferView<const T>& input, const BufferView<T>& output) noexcept
    {
        const auto numSamplesIn = input.getNumSamples();
        const auto numSamplesOut = output.getNumSamples();
        const auto newNumLeftoverSamples = numSamplesIn + numSamplesLeftoverOut - numSamplesOut;
        jassert (newNumLeftoverSamples <= M);

        auto leftoverSamplesToUse = std::min (numSamplesLeftoverOut, numSamplesOut);

        for (auto [ch, outData] : buffer_iters::channels (output))
        {
            const auto inputs = input.getReadSpan (ch);
            const auto leftovers = getLeftoversOut (ch);

            std::copy (leftovers.begin(), leftovers.begin() + leftoverSamplesToUse, outData.begin());
            std::copy (inputs.begin(), inputs.begin() + numSamplesOut - leftoverSamplesToUse, outData.begin() + leftoverSamplesToUse);

            if (leftoverSamplesToUse < numSamplesLeftoverOut)
            {
                std::copy (leftovers.begin() + leftoverSamplesToUse, leftovers.end(), leftovers.begin());
            }

            std::copy (inputs.begin() + numSamplesOut - leftoverSamplesToUse,
                       inputs.end(),
                       leftovers.begin() + numSamplesLeftoverOut - leftoverSamplesToUse);
        }

        numSamplesLeftoverOut = newNumLeftoverSamples;
    }

private:
    nonstd::span<T> getLeftoversIn (int ch)
    {
        return { leftoverDataIn.data() + ch * M, static_cast<size_t> (M) };
    }

    nonstd::span<T> getLeftoversOut (int ch)
    {
        return { leftoverDataOut.data() + ch * M, static_cast<size_t> (M) };
    }

    int M = 0;
    int numChannels = 0;
    int numSamplesLeftoverIn = 0;
    int numSamplesLeftoverOut = -1;

    static constexpr size_t maxMultiple = 8;
    std::array<T, CHOWDSP_BUFFER_MAX_NUM_CHANNELS * maxMultiple> leftoverDataIn {};
    std::array<T, CHOWDSP_BUFFER_MAX_NUM_CHANNELS * maxMultiple> leftoverDataOut {};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BufferMultiple)
};
} // namespace chowdsp
