#include <CatchUtils.h>
#include <chowdsp_dsp_data_structures/chowdsp_dsp_data_structures.h>

static_assert (! chowdsp::IsConstBufferType<chowdsp::Buffer<float>>);
static_assert (! chowdsp::IsConstBufferType<chowdsp::Buffer<float>&>);
static_assert (chowdsp::IsConstBufferType<const chowdsp::Buffer<float>>);
static_assert (chowdsp::IsConstBufferType<const chowdsp::Buffer<float>&>);

static_assert (! chowdsp::IsConstBufferType<chowdsp::BufferView<float>>);
static_assert (! chowdsp::IsConstBufferType<chowdsp::BufferView<float>&>);
static_assert (! chowdsp::IsConstBufferType<const chowdsp::BufferView<float>>);
static_assert (! chowdsp::IsConstBufferType<const chowdsp::BufferView<float>&>);
static_assert (chowdsp::IsConstBufferType<const chowdsp::BufferView<const float>>);
static_assert (chowdsp::IsConstBufferType<const chowdsp::BufferView<const float>&>);

TEMPLATE_TEST_CASE ("Buffer Iterators Test",
                    "[dsp][buffers][simd]",
                    (chowdsp::Buffer<float>),
                    (chowdsp::Buffer<double>),
                    (chowdsp::Buffer<xsimd::batch<float>>),
                    (chowdsp::Buffer<xsimd::batch<double>>),
                    (chowdsp::StaticBuffer<float, 2, 96>),
                    (chowdsp::StaticBuffer<double, 2, 96>),
                    (chowdsp::StaticBuffer<xsimd::batch<float>, 2, 96>),
                    (chowdsp::StaticBuffer<xsimd::batch<double>, 2, 96>),
                    (juce::AudioBuffer<float>),
                    (juce::AudioBuffer<double>) )
{
    using BufferType = TestType;
    using SampleType = chowdsp::BufferMath::detail::BufferSampleType<BufferType>;

    SECTION ("Channels")
    {
        BufferType buffer { 2, 4 };

        {
            int count = 0;
            for (auto [channel, data] : chowdsp::buffer_iters::channels (buffer))
            {
                for (auto& x_n : data)
                    x_n = (SampleType) static_cast<float> (count++);
            }
        }

        int count = 0;
        for (auto [channel, data] : chowdsp::buffer_iters::channels (chowdsp::asConstBuffer (buffer)))
        {
            for (auto& x_n : data)
                REQUIRE (chowdsp::SIMDUtils::all ((SampleType) static_cast<float> (count++) == x_n));
        }
    }

    struct SubBlocksTester
    {
        int channel;
        int sample;
        int size;
        const SampleType* data;
    };

    const auto testBufferSubBlocks = [] (auto& b, const auto& testers, const auto channelWise)
    {
        size_t test_count = 0;
        for (auto [ch, n, sub_block] : chowdsp::buffer_iters::sub_blocks<32, channelWise> (b))
        {
            REQUIRE (ch == testers[test_count].channel);
            REQUIRE (n == testers[test_count].sample);
            REQUIRE ((int) sub_block.size() == testers[test_count].size);
            REQUIRE (sub_block.data() == testers[test_count].data);
            ++test_count;
        }
    };

    SECTION ("Sub-Blocks (Sample-wise)")
    {
        BufferType buffer { 2, 70 };

        const std::array<SubBlocksTester, 6> testers {
            SubBlocksTester {0, 0, 32, buffer.getWritePointer (0)},
            SubBlocksTester {0, 32, 32, buffer.getWritePointer (0) + 32},
            SubBlocksTester {0, 64, 6, buffer.getWritePointer (0) + 64},
            SubBlocksTester {1, 0, 32, buffer.getWritePointer (1)},
            SubBlocksTester {1, 32, 32, buffer.getWritePointer (1) + 32},
            SubBlocksTester {1, 64, 6, buffer.getWritePointer (1) + 64},
        };

        testBufferSubBlocks (buffer, testers, std::integral_constant<bool, false>{});
        testBufferSubBlocks (chowdsp::asConstBuffer (buffer), testers, std::integral_constant<bool, false>{});
    }

    SECTION ("Sub-Blocks (Channel-wise)")
    {
        BufferType buffer { 2, 70 };

        const std::array<SubBlocksTester, 6> testers {
            SubBlocksTester {0, 0, 32, buffer.getWritePointer (0)},
            SubBlocksTester {1, 0, 32, buffer.getWritePointer (1)},
            SubBlocksTester {0, 32, 32, buffer.getWritePointer (0) + 32},
            SubBlocksTester {1, 32, 32, buffer.getWritePointer (1) + 32},
            SubBlocksTester {0, 64, 6, buffer.getWritePointer (0) + 64},
            SubBlocksTester {1, 64, 6, buffer.getWritePointer (1) + 64},
        };

        testBufferSubBlocks (buffer, testers, std::integral_constant<bool, true>{});
        testBufferSubBlocks (chowdsp::asConstBuffer (buffer), testers, std::integral_constant<bool, true>{});
    }
}
