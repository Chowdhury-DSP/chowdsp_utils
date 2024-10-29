#include <CatchUtils.h>
#include <chowdsp_dsp_data_structures/chowdsp_dsp_data_structures.h>

TEST_CASE ("Buffer Multiple Test", "[dsp][data-structures]")
{
    static constexpr int numChannels = 1;
    static constexpr int bufferSize = 64;
    static constexpr int N = 1000;
    std::vector<float> data (N, 0.0f);
    std::iota (std::begin (data), std::end (data), 0.0f);

    chowdsp::ArenaAllocator<std::array<std::byte, bufferSize * sizeof (float)>> arena {};

    for (int multiple : std::initializer_list<int> { 2 }) //, 3, 4, 5, 6, 7 })
    {
        chowdsp::BufferMultiple<float> multipleProcessor {};
        multipleProcessor.prepare ({ 0.0, (uint32_t) bufferSize, (uint32_t) numChannels }, multiple);

        int inCount = 0;
        int mCount = 0;
        int outCount = 0;
        for (int n : std::initializer_list<int> { 6, 20, 1, 3, 15, 16, 5, 7, 18 })
        {
            chowdsp::BufferView buffer { data.data(), n, inCount };
            const auto mBuffer = multipleProcessor.processBufferIn (arena, buffer);

            REQUIRE (mBuffer.getNumSamples() % multiple == 0);
            for (auto [idx, x] : chowdsp::enumerate (mBuffer.getWriteSpan (0)))
            {
                REQUIRE (x == std::max (static_cast<float> (mCount + (int) idx - (multiple - 1)), 0.0f));
            }

            inCount += n;
            mCount += mBuffer.getNumSamples();
            arena.clear();
        }
    }
}
