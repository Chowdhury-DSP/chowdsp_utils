#include <chowdsp_buffers/chowdsp_buffers.h>
#include <chowdsp_data_structures/chowdsp_data_structures.h>

JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wfloat-equal")
#include <CatchUtils.h>

template <typename BufferViewType, typename T>
void testBufferView (const BufferViewType view, const chowdsp::Buffer<T>& buffer)
{
    using chowdsp::SIMDUtils::all;

    REQUIRE_MESSAGE (buffer.getNumChannels() == view.getNumChannels(), "View has the incorrect number of channels!");
    REQUIRE_MESSAGE (buffer.getNumSamples() == view.getNumSamples(), "View has the incorrect number of samples!");

    for (auto [ch, x] : chowdsp::buffer_iters::channels (buffer))
    {
        const auto* xView = view.getReadPointer (ch);
        for (auto [n, x_n] : chowdsp::enumerate (x))
            REQUIRE_MESSAGE (all (x_n == xView[n]), "Data is not correct!");
    }
}

template <typename BufferViewType, typename T>
void testBufferViewOffset (const BufferViewType view, const chowdsp::StaticBuffer<T, 2, 1024>& buffer, int offset)
{
    using chowdsp::SIMDUtils::all;

    REQUIRE_MESSAGE (buffer.getNumChannels() == view.getNumChannels(), "View has the incorrect number of channels!");
    REQUIRE_MESSAGE (view.getNumSamples() == buffer.getNumSamples() - offset, "View has the incorrect number of samples!");

    for (auto [ch, x] : chowdsp::buffer_iters::channels (buffer))
    {
        const auto* xView = view.getReadPointer (ch);
        for (auto& x_n : x.subspan (0, (size_t) offset))
            REQUIRE_MESSAGE (all (x_n == T (0)), "Skipped data is not correct!");

        for (auto [n, x_n] : chowdsp::enumerate (x.subspan ((size_t) offset)))
            REQUIRE_MESSAGE (all (x_n == xView[n]), "Data in view not correct!");
    }
}

TEMPLATE_TEST_CASE ("Buffer View Test", "[dsp][buffers][simd]", float, double, xsimd::batch<float>, xsimd::batch<double>)
{
    using chowdsp::SIMDUtils::all;
    auto minus1To1 = test_utils::RandomFloatGenerator { -1.0f, 1.0f };

    SECTION ("Data Test")
    {
        chowdsp::Buffer<TestType> buffer { 2, 128 };
        for (auto [_, x] : chowdsp::buffer_iters::channels (buffer))
        {
            for (auto& x_n : x)
                x_n = TestType (minus1To1());
        }

        testBufferView<chowdsp::BufferView<TestType>> (buffer, buffer);
        testBufferView<chowdsp::BufferView<const TestType>> (buffer, buffer);
        testBufferView<chowdsp::BufferView<const TestType>> (static_cast<const chowdsp::Buffer<TestType>&> (buffer), buffer);

        testBufferView<chowdsp::BufferView<TestType>> ({ buffer.getArrayOfWritePointers(), buffer.getNumChannels(), buffer.getNumSamples() }, buffer);
        testBufferView<chowdsp::BufferView<const TestType>> ({ buffer.getArrayOfWritePointers(), buffer.getNumChannels(), buffer.getNumSamples() }, buffer);
        testBufferView<chowdsp::BufferView<const TestType>> ({ buffer.getArrayOfReadPointers(), buffer.getNumChannels(), buffer.getNumSamples() }, buffer);

        const chowdsp::BufferView<const TestType> view { buffer };
        testBufferView<chowdsp::BufferView<const TestType>> ({ view, 0 }, buffer);
    }

    SECTION ("Data Offset Test")
    {
        static constexpr int offset = 32;

        chowdsp::StaticBuffer<TestType, 2, 1024> buffer { 2, 128 };
        chowdsp::BufferView<TestType> view { buffer, offset };

        for (auto [_, x] : chowdsp::buffer_iters::channels (view))
        {
            for (auto& x_n : x)
                x_n = TestType (minus1To1());
        }

        const auto& constBuffer = std::as_const (buffer);
        testBufferViewOffset<chowdsp::BufferView<TestType>> ({ buffer, offset }, buffer, offset);
        testBufferViewOffset<chowdsp::BufferView<const TestType>> ({ buffer, offset }, buffer, offset);
        testBufferViewOffset<chowdsp::BufferView<const TestType>> ({ constBuffer, offset }, buffer, offset);
    }

    SECTION ("Clear Test")
    {
        chowdsp::Buffer<TestType> buffer { 2, 128 };
        chowdsp::BufferView<TestType> view { buffer };

        for (auto [_, xWrite] : chowdsp::buffer_iters::channels (view))
        {
            for (auto& x_n : xWrite)
                x_n = TestType (minus1To1());
        }

        view.clear();
        for (const auto [_, xRead] : chowdsp::buffer_iters::channels (view))
        {
            for (auto& x_n : xRead)
                REQUIRE_MESSAGE (all (x_n == TestType (0)), "Data in view not correct!");
        }
    }

    SECTION ("1D-Array Test")
    {
        const auto testBuffer = [] (const auto& buffer, float offset = 0.0f)
        {
            REQUIRE (buffer.getNumChannels() == 1);
            REQUIRE (buffer.getNumSamples() == 5);

            for (int i = 0; i < 5; ++i)
                REQUIRE (buffer.getReadPointer (0)[i] == (float) i + offset);
        };

        float data[10] = { 0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f };
        {
            testBuffer (chowdsp::BufferView<float> { data, 5 });
            testBuffer (chowdsp::BufferView<const float> { data, 5 });
        }
        {
            testBuffer (chowdsp::BufferView<float> { data, 5, 5 }, 5.0f);
            testBuffer (chowdsp::BufferView<const float> { data, 5, 5 }, 5.0f);
        }
    }

    SECTION ("Copy Constructor Test")
    {
        chowdsp::Buffer<TestType> buffer { 1, 32 };
        chowdsp::BufferView<TestType> buffer_view { buffer };

        {
            chowdsp::BufferView<TestType> buffer_view_copy { buffer_view }; // NOLINT
            REQUIRE (buffer_view.getReadPointer (0) == buffer_view_copy.getReadPointer (0));
        }

        {
            chowdsp::BufferView<TestType> buffer_view_copy = buffer_view; // NOLINT
            REQUIRE (buffer_view.getReadPointer (0) == buffer_view_copy.getReadPointer (0));
        }
    }

    if constexpr (std::is_floating_point_v<TestType>)
    {
        SECTION ("make_temp_buffer")
        {
            chowdsp::ArenaAllocator<> arena { 1 << 10 };
            chowdsp::make_temp_buffer<float> (arena, 2, 61);
            REQUIRE (arena.get_bytes_used() == 64 * 2 * sizeof (float));
        }
    }
}
JUCE_END_IGNORE_WARNINGS_GCC_LIKE

TEMPLATE_TEST_CASE ("Buffer View Template Deduction Test", "[dsp][buffers][simd]", float, double, xsimd::batch<float>, xsimd::batch<double>)
{
    SECTION ("1D Raw Data")
    {
        TestType* data = nullptr;
        chowdsp::BufferView buffer_view { data, 0 };
        STATIC_REQUIRE (std::is_same_v<decltype (buffer_view), chowdsp::BufferView<TestType>>);

        const TestType* data_const = nullptr;
        chowdsp::BufferView buffer_view_const { data_const, 0 };
        STATIC_REQUIRE (std::is_same_v<decltype (buffer_view_const), chowdsp::BufferView<const TestType>>);
    }

    SECTION ("2D Raw Data")
    {
        TestType* const* data = nullptr;
        chowdsp::BufferView buffer_view { data, 0, 0 };
        STATIC_REQUIRE (std::is_same_v<decltype (buffer_view), chowdsp::BufferView<TestType>>);

        const TestType* const* data_const = nullptr;
        chowdsp::BufferView buffer_view_const { data_const, 0, 0 };
        STATIC_REQUIRE (std::is_same_v<decltype (buffer_view_const), chowdsp::BufferView<const TestType>>);
    }

    SECTION ("chowdsp::Buffer")
    {
        chowdsp::Buffer<TestType> buffer;
        chowdsp::BufferView buffer_view { buffer };
        STATIC_REQUIRE (std::is_same_v<decltype (buffer_view), chowdsp::BufferView<TestType>>);

        chowdsp::BufferView buffer_view_const { std::as_const (buffer), 0, -1, 0, -1 };
        STATIC_REQUIRE (std::is_same_v<decltype (buffer_view_const), chowdsp::BufferView<const TestType>>);
    }

    SECTION ("chowdsp::StaticBuffer")
    {
        chowdsp::StaticBuffer<TestType, 1, 1> buffer;
        chowdsp::BufferView buffer_view { buffer };
        STATIC_REQUIRE (std::is_same_v<decltype (buffer_view), chowdsp::BufferView<TestType>>);

        const chowdsp::StaticBuffer<TestType, 1, 1> buffer_const;
        chowdsp::BufferView buffer_view_const { buffer_const };
        STATIC_REQUIRE (std::is_same_v<decltype (buffer_view_const), chowdsp::BufferView<const TestType>>);
    }

    if constexpr (std::is_floating_point_v<TestType>)
    {
        SECTION ("juce::AudioBuffer")
        {
            juce::AudioBuffer<TestType> buffer;
            chowdsp::BufferView buffer_view { buffer };
            STATIC_REQUIRE (std::is_same_v<decltype (buffer_view), chowdsp::BufferView<TestType>>);

            chowdsp::BufferView buffer_view_const { std::as_const (buffer), 0, -1, 0, -1 };
            STATIC_REQUIRE (std::is_same_v<decltype (buffer_view_const), chowdsp::BufferView<const TestType>>);
        }

        SECTION ("juce::dsp::AudioBlock")
        {
            juce::dsp::AudioBlock<TestType> buffer;
            chowdsp::BufferView buffer_view { buffer };
            STATIC_REQUIRE (std::is_same_v<decltype (buffer_view), chowdsp::BufferView<TestType>>);

            juce::dsp::AudioBlock<const TestType> buffer_const;
            chowdsp::BufferView buffer_view_const { std::as_const (buffer_const), 0, -1, 0, -1 };
            STATIC_REQUIRE (std::is_same_v<decltype (buffer_view_const), chowdsp::BufferView<const TestType>>);
        }
    }
}
