#include <chowdsp_dsp_data_structures/chowdsp_dsp_data_structures.h>

JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wfloat-equal")
#include <CatchUtils.h>

static_assert (! chowdsp::IsConstBuffer<chowdsp::Buffer<float>>);
static_assert (! chowdsp::IsConstBuffer<chowdsp::Buffer<float>&>);
static_assert (chowdsp::IsConstBuffer<const chowdsp::Buffer<float>>);
static_assert (chowdsp::IsConstBuffer<const chowdsp::Buffer<float>&>);

static_assert (! chowdsp::IsConstBuffer<chowdsp::BufferView<float>>);
static_assert (! chowdsp::IsConstBuffer<chowdsp::BufferView<float>&>);
static_assert (! chowdsp::IsConstBuffer<const chowdsp::BufferView<float>>);
static_assert (! chowdsp::IsConstBuffer<const chowdsp::BufferView<float>&>);
static_assert (chowdsp::IsConstBuffer<const chowdsp::BufferView<const float>>);
static_assert (chowdsp::IsConstBuffer<const chowdsp::BufferView<const float>&>);

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
    using SampleType = chowdsp::BufferSampleType<BufferType>;

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
        for (auto [channel, data] : chowdsp::buffer_iters::channels (std::as_const (buffer)))
        {
            for (auto& x_n : data)
                REQUIRE (chowdsp::SIMDUtils::all ((SampleType) static_cast<float> (count++) == x_n));
        }
    }

    SECTION ("Samples")
    {
        BufferType buffer { 2, 4 };

        {
            int count = 0;
            for (auto [sample, data] : chowdsp::buffer_iters::samples (buffer))
            {
                for (auto& x_n : data)
                    x_n = (SampleType) static_cast<float> (count++);
            }
        }

        int count = 0;
        for (auto [sample, data] : chowdsp::buffer_iters::samples (std::as_const (buffer)))
        {
            for (auto& x_n : data)
                REQUIRE (chowdsp::SIMDUtils::all ((SampleType) static_cast<float> (count++) == x_n));
        }
    }

    SECTION ("Zip Channels")
    {
        BufferType buffer1 { 2, 4 };
        BufferType buffer2 { 1, 2 };

        {
            int count = 0;
            for (auto [channel, data1, data2] : chowdsp::buffer_iters::zip_channels (buffer1, buffer2))
            {
                REQUIRE (channel == 0);
                REQUIRE (data1.size() == 4);
                REQUIRE (data2.size() == 2);
                for (auto [x_n, y_n] : chowdsp::zip (data1, data2))
                {
                    x_n = (SampleType) static_cast<float> (count);
                    y_n = (SampleType) static_cast<float> (count);
                    ++count;
                }
            }
        }

        int count = 0;
        for (auto [channel, data1, data2] : chowdsp::buffer_iters::zip_channels (std::as_const (buffer1), std::as_const (buffer2)))
        {
            for (auto [x_n, y_n] : chowdsp::zip (data1, data2))
            {
                REQUIRE (chowdsp::SIMDUtils::all ((SampleType) static_cast<float> (count) == x_n));
                REQUIRE (chowdsp::SIMDUtils::all (x_n == y_n));
                ++count;
            }
        }
    }

    if constexpr (std::is_same_v<BufferType, chowdsp::Buffer<float>>)
    {
        SECTION ("Buffer View Channels")
        {
            BufferType buffer { 2, 4 };
            const chowdsp::BufferView<SampleType> bufferView { buffer };
            const chowdsp::BufferView<const SampleType> constBufferView { buffer };

            {
                int count = 0;
                for (auto [channel, data] : chowdsp::buffer_iters::channels (bufferView))
                {
                    for (auto& x_n : data)
                        x_n = (SampleType) static_cast<float> (count++);
                }
            }

            int count = 0;
            for (const auto [channel, data] : chowdsp::buffer_iters::channels (constBufferView))
            {
                REQUIRE ((int) data.size() == bufferView.getNumSamples());
                for (auto& x_n : data)
                {
                    REQUIRE (chowdsp::SIMDUtils::all ((SampleType) static_cast<float> (count) == x_n));
                    ++count;
                }
            }
        }
    }

    SECTION ("Buffer View Zip Channels")
    {
        BufferType buffer1 { 2, 4 };
        BufferType buffer2 { 1, 2 };
        const chowdsp::BufferView<SampleType> bufferView1 { buffer1 };
        const chowdsp::BufferView<const SampleType> constBufferView1 { buffer1 };
        const chowdsp::BufferView<SampleType> bufferView2 { buffer2 };
        const chowdsp::BufferView<const SampleType> constBufferView2 { buffer2 };

        {
            int count = 0;
            for (auto [channel, data1, data2] : chowdsp::buffer_iters::zip_channels (bufferView1, bufferView2))
            {
                REQUIRE (channel == 0);
                REQUIRE (data1.size() == 4);
                REQUIRE (data2.size() == 2);
                for (auto [x_n, y_n] : chowdsp::zip (data1, data2))
                {
                    x_n = (SampleType) static_cast<float> (count);
                    y_n = (SampleType) static_cast<float> (count);
                    ++count;
                }
            }
        }

        int count = 0;
        for (auto [channel, data1, data2] : chowdsp::buffer_iters::zip_channels (std::as_const (constBufferView1),
                                                                                 std::as_const (constBufferView2)))
        {
            for (auto [x_n, y_n] : chowdsp::zip (data1, data2))
            {
                REQUIRE (chowdsp::SIMDUtils::all ((SampleType) static_cast<float> (count) == x_n));
                REQUIRE (chowdsp::SIMDUtils::all (x_n == y_n));
                ++count;
            }
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
            SubBlocksTester { 0, 0, 32, buffer.getWritePointer (0) },
            SubBlocksTester { 0, 32, 32, buffer.getWritePointer (0) + 32 },
            SubBlocksTester { 0, 64, 6, buffer.getWritePointer (0) + 64 },
            SubBlocksTester { 1, 0, 32, buffer.getWritePointer (1) },
            SubBlocksTester { 1, 32, 32, buffer.getWritePointer (1) + 32 },
            SubBlocksTester { 1, 64, 6, buffer.getWritePointer (1) + 64 },
        };

        testBufferSubBlocks (buffer, testers, std::integral_constant<bool, false> {});
        testBufferSubBlocks (std::as_const (buffer), testers, std::integral_constant<bool, false> {});
    }

    SECTION ("Sub-Blocks (Channel-wise)")
    {
        BufferType buffer { 2, 70 };

        const std::array<SubBlocksTester, 6> testers {
            SubBlocksTester { 0, 0, 32, buffer.getWritePointer (0) },
            SubBlocksTester { 1, 0, 32, buffer.getWritePointer (1) },
            SubBlocksTester { 0, 32, 32, buffer.getWritePointer (0) + 32 },
            SubBlocksTester { 1, 32, 32, buffer.getWritePointer (1) + 32 },
            SubBlocksTester { 0, 64, 6, buffer.getWritePointer (0) + 64 },
            SubBlocksTester { 1, 64, 6, buffer.getWritePointer (1) + 64 },
        };

        testBufferSubBlocks (buffer, testers, std::integral_constant<bool, true> {});
        testBufferSubBlocks (std::as_const (buffer), testers, std::integral_constant<bool, true> {});
    }

    if constexpr (std::is_same_v<BufferType, chowdsp::Buffer<float>>)
    {
        SECTION ("Buffer View Sub-Blocks")
        {
            BufferType buffer { 2, 70 };
            const chowdsp::BufferView<SampleType> bufferView { buffer };
            const chowdsp::BufferView<const SampleType> constBufferView { buffer };

            const std::array<SubBlocksTester, 6> testers {
                SubBlocksTester { 0, 0, 32, buffer.getWritePointer (0) },
                SubBlocksTester { 1, 0, 32, buffer.getWritePointer (1) },
                SubBlocksTester { 0, 32, 32, buffer.getWritePointer (0) + 32 },
                SubBlocksTester { 1, 32, 32, buffer.getWritePointer (1) + 32 },
                SubBlocksTester { 0, 64, 6, buffer.getWritePointer (0) + 64 },
                SubBlocksTester { 1, 64, 6, buffer.getWritePointer (1) + 64 },
            };

            testBufferSubBlocks (bufferView, testers, std::integral_constant<bool, true> {});
            testBufferSubBlocks (constBufferView, testers, std::integral_constant<bool, true> {});
        }
    }
}
JUCE_END_IGNORE_WARNINGS_GCC_LIKE
