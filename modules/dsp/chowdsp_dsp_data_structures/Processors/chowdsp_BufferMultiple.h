#pragma once

namespace chowdsp
{
template <typename T>
class BufferMultiple
{
public:
    BufferMultiple() = default;

    int prepare (const juce::dsp::ProcessSpec& spec, int multiple)
    {
        jassert (multiple > 1 && multiple <= static_cast<int> (maxMultiple));
        M = multiple;
        numChannels = static_cast<int> (spec.numChannels);

        reset();

        return Math::round_to_next_multiple (Math::round_to_next_multiple (static_cast<int> (spec.maximumBlockSize), M),
                                             static_cast<int> (SIMDUtils::defaultSIMDAlignment / sizeof (T)));
    }

    void reset() noexcept
    {
        leftoverDataIn.fill (T {});
        leftoverDataOut.fill (T {});
        numSamplesLeftoverIn = M - 1;
        numSamplesLeftoverOut = 1;
    }

    [[nodiscard]] int getMultipliedBufferLatency() const noexcept
    {
        return M - 1;
    }

    [[nodiscard]] int getRoundTripLatency() const noexcept
    {
        return M;
    }

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
