#include <CatchUtils.h>
#include <chowdsp_buffers/chowdsp_buffers.h>

template <typename T>
using TestStaticBuffer = chowdsp::StaticBuffer<T, 2, 128>;

TEMPLATE_PRODUCT_TEST_CASE ("Buffer Test", "[dsp][buffers][simd]", (chowdsp::Buffer, TestStaticBuffer), (float, double, xsimd::batch<float>, xsimd::batch<double>) )
{
    using BufferType = TestType;
    using SampleType = typename BufferType::Type;

    using chowdsp::SIMDUtils::all;
    auto minus1To1 = test_utils::RandomFloatGenerator { -1.0f, 0.0f };

    SECTION ("Default Construction Test")
    {
        BufferType buffer;
        REQUIRE_MESSAGE (buffer.getNumChannels() == 0, "Default constructed # channels is incorrect");
        REQUIRE_MESSAGE (buffer.getNumSamples() == 0, "Default constructed # samples is incorrect");
    }

    SECTION ("Construct with size Test")
    {
        BufferType buffer { 2, 128 };
        REQUIRE_MESSAGE (buffer.getNumChannels() == 2, "Constructed buffer with 2 channels is incorrect");
        REQUIRE_MESSAGE (buffer.getNumSamples() == 128, "Constructed buffer with 128 samples is incorrect");
    }

    SECTION ("Resize Test")
    {
        BufferType buffer;

        buffer.setMaxSize (2, 128);
        REQUIRE_MESSAGE (buffer.getNumChannels() == 2, "Resized # channels is incorrect");
        REQUIRE_MESSAGE (buffer.getNumSamples() == 128, "Resized # samples is incorrect");
    }

    SECTION ("Move Construct Test")
    {
        BufferType buffer { 2, 128 };
        buffer.getWritePointer (0)[100] = SampleType (1);
        buffer.getWritePointer (1)[110] = SampleType (-1);

        auto&& movedBuffer = std::move (buffer);
        REQUIRE_MESSAGE (movedBuffer.getNumChannels() == 2, "Moved buffer with 2 channels is incorrect");
        REQUIRE_MESSAGE (movedBuffer.getNumSamples() == 128, "Moved buffer with 128 samples is incorrect");
        REQUIRE_MESSAGE (all (buffer.getReadPointer (0)[0] == SampleType (0)), "Cleared sample in channel 0 is incorrect");
        REQUIRE_MESSAGE (all (buffer.getReadPointer (0)[100] == SampleType (1)), "Set sample in channel 0 is incorrect");
        REQUIRE_MESSAGE (all (buffer.getReadPointer (1)[0] == SampleType (0)), "Cleared sample in channel 1 is incorrect");
        REQUIRE_MESSAGE (all (buffer.getReadPointer (1)[110] == SampleType (-1)), "Set sample in channel 1 is incorrect");
    }

    SECTION ("Clear Test")
    {
        BufferType buffer { 2, 128 };
        for (auto [_, x] : chowdsp::buffer_iters::channels (buffer))
        {
            for (auto& x_n : x)
                x_n = SampleType (minus1To1());
        }

        for (int i = 0; i < 2; ++i)
        {
            buffer.clear();
            for (auto [_, x] : chowdsp::buffer_iters::channels (buffer))
            {
                for (auto& x_n : x)
                    REQUIRE_MESSAGE (all (x_n == SampleType (0)), "Buffer was not cleared!");
            }
        }
    }

    SECTION ("Resize Test")
    {
        BufferType buffer { 2, 128 };
        for (auto [_, channelData] : chowdsp::buffer_iters::channels (buffer))
        {
            for (auto& sample : channelData)
                sample = SampleType (minus1To1());
        }

        buffer.setCurrentSize (1, 32);
        buffer.clear();

        buffer.setCurrentSize (2, 64);
        for (auto [ch, channelData] : chowdsp::buffer_iters::channels (buffer))
        {
            for (auto [n, sample] : chowdsp::enumerate (channelData))
                REQUIRE_MESSAGE (all (sample == SampleType (0)), "Buffer was not cleared! " << ch << ", " << n);
        }
    }

    SECTION ("Resize Null Test")
    {
        BufferType buffer { 2, 128 };

        buffer.setCurrentSize (0, 64);
        REQUIRE (buffer.getNumChannels() == 0);
        REQUIRE (buffer.getNumSamples() == 64);
        REQUIRE (buffer.getArrayOfReadPointers() != nullptr);

        buffer.setCurrentSize (1, 0);
        REQUIRE (buffer.getNumChannels() == 1);
        REQUIRE (buffer.getNumSamples() == 0);
        REQUIRE (buffer.getArrayOfReadPointers() != nullptr);
    }

    SECTION ("Alignment Test")
    {
        const BufferType buffer { 2, 33 };

        for (auto [_, channelData] : chowdsp::buffer_iters::channels (buffer))
            REQUIRE (chowdsp::SIMDUtils::isAligned (channelData.data()));
    }
}
