#include <CatchUtils.h>
#include <chowdsp_dsp_data_structures/chowdsp_dsp_data_structures.h>

#include <iostream>

TEST_CASE ("Buffer Multiple Test", "[dsp][data-structures]")
{
    static constexpr int numChannels = 2;
    static constexpr int bufferSize = 64;
    static constexpr int N = 1000;

    for (int multiple : std::initializer_list<int> { 2, 3, 4, 5, 6, 7 })
    {
        std::vector<float> data (N, 0.0f);
        std::iota (std::begin (data), std::end (data), 0.0f);
        std::vector<float> data2 { data.begin(), data.end() };
        float* dataPtrs[numChannels] = { data.data(), data2.data() };

        chowdsp::BufferMultiple<float> multipleProcessor {};
        int maxMultipleBufferSize = multipleProcessor.prepare ({ 0.0, (uint32_t) bufferSize, (uint32_t) numChannels }, multiple);
        const auto mLatencySamples = multipleProcessor.getMultipleBufferLatency();
        const auto roundTripLatencySamples = multipleProcessor.getRoundTripLatency();

        chowdsp::ArenaAllocator<> arena { numChannels * maxMultipleBufferSize * sizeof (float) };

        int inOutCount = 0;
        int mCount = 0;
        for (int n : std::initializer_list<int> { 6, 20, 1, 3, 15, 16, 5, 7, 18, bufferSize, bufferSize, bufferSize })
        {
            chowdsp::BufferView buffer { dataPtrs, numChannels, n, inOutCount };
            const auto mBuffer = multipleProcessor.processBufferIn (arena, buffer);

            REQUIRE (mBuffer.getNumSamples() % multiple == 0);
            REQUIRE (mBuffer.getNumSamples() <= maxMultipleBufferSize);
            for (auto [ch, channelData] : chowdsp::buffer_iters::channels (mBuffer))
            {
                for (const auto& [idx, x] : chowdsp::enumerate (channelData))
                {
                    REQUIRE (x == std::max (static_cast<float> (mCount + (int) idx - mLatencySamples), 0.0f));
                    x = -x;
                }
            }

            multipleProcessor.processBufferOut (mBuffer, buffer);
            for (auto [ch, channelData] : chowdsp::buffer_iters::channels (buffer))
            {
                for (const auto& [idx, x] : chowdsp::enumerate (channelData))
                {
                    REQUIRE (-x == std::max (static_cast<float> (inOutCount + (int) idx - roundTripLatencySamples), 0.0f));
                }
            }

            inOutCount += n;
            mCount += mBuffer.getNumSamples();
            arena.clear();
        }
    }
}
