#pragma once

namespace chowdsp
{
template <typename T>
class BufferMultiple
{
public:
    BufferMultiple() = default;

    void prepare (const juce::dsp::ProcessSpec& spec, int multiple)
    {
        jassert (multiple > 1 && multiple < 8);
        M = multiple;

        maxBufferSize = static_cast<int> (spec.maximumBlockSize);
        numChannels = static_cast<int> (spec.numChannels);

        reset();
    }

    void reset() noexcept
    {
        data.fill (T {});
        numSamplesLeftover = M - 1;
    }

    BufferView<T> processBufferIn (ArenaAllocatorView arena, const BufferView<const T>& input) noexcept
    {
        const auto numSamplesIn = input.getNumSamples();
        jassert (numSamplesIn >= 2 * M);

        const auto numMultiplesOut = std::min (numSamplesIn + numSamplesLeftover - (M - 1), maxBufferSize) / M;
        const auto numSamplesOut = M * numMultiplesOut;
        const auto newNumLeftoverSamples = numSamplesIn + numSamplesLeftover - numSamplesOut;
        jassert (newNumLeftoverSamples <= M);

        auto leftoverSamplesToUse = std::min (numSamplesLeftover, numSamplesOut);

        const auto bufferOut = make_temp_buffer<T> (arena, input.getNumChannels(), numSamplesOut);

        for (auto [ch, outData] : buffer_iters::channels (bufferOut))
        {
            const auto inputs = input.getReadSpan (ch);
            const auto leftovers = getLeftovers (ch);

            std::copy (leftovers.begin(), leftovers.begin() + leftoverSamplesToUse, outData.begin());
            std::copy (inputs.begin(), inputs.begin() + numSamplesOut - leftoverSamplesToUse, outData.begin() + leftoverSamplesToUse);

            if (leftoverSamplesToUse < numSamplesLeftover)
            {
                std::copy (leftovers.begin() + leftoverSamplesToUse, leftovers.end(), leftovers.begin());
            }

            std::copy (inputs.begin() + numSamplesOut - leftoverSamplesToUse,
                       inputs.end(),
                       leftovers.begin() + numSamplesLeftover - leftoverSamplesToUse);
            numSamplesLeftover = newNumLeftoverSamples;
        }

        return bufferOut;
    }

    void processBufferOut (const BufferView<const T>& input, const BufferView<T>& output) noexcept;

private:
    nonstd::span<T> getLeftovers (int ch)
    {
        return { data.data() + ch * M, static_cast<size_t> (M) };
    }

    int M = 0;
    int maxBufferSize = 0;
    int numChannels = 0;
    int numSamplesLeftover = 0;
    std::array<T, CHOWDSP_BUFFER_MAX_NUM_CHANNELS * 16> data {};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BufferMultiple)
};
} // namespace chowdsp
