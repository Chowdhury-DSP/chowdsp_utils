#include <CatchUtils.h>
#include <chowdsp_dsp_data_structures/chowdsp_dsp_data_structures.h>

template <typename BufferViewType, typename T>
void testBufferView (const BufferViewType view, const chowdsp::Buffer<T>& buffer)
{
    using chowdsp::SIMDUtils::all;

    REQUIRE_MESSAGE (buffer.getNumChannels() == view.getNumChannels(), "View has the incorrect number of channels!");
    REQUIRE_MESSAGE (buffer.getNumSamples() == view.getNumSamples(), "View has the incorrect number of samples!");

    for (auto [ch, x] : chowdsp::buffer_iters::channels (buffer))
    {
        const auto* xView = view.getReadPointer (ch);
        for (int n = 0; n < buffer.getNumSamples(); ++n)
            REQUIRE_MESSAGE (all (x[n] == xView[n]), "Data is not correct!");
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
        for (int n = 0; n < offset; ++n)
            REQUIRE_MESSAGE (all (x[n] == T (0)), "Skipped data is not correct!");

        for (int n = offset; n < buffer.getNumSamples(); ++n)
            REQUIRE_MESSAGE (all (x[n] == xView[n - offset]), "Data in view not correct!");
    }
}

TEMPLATE_TEST_CASE ("Buffer View Test", "[dsp][buffers][simd]", float, double, xsimd::batch<float>, xsimd::batch<double>)
{
    using chowdsp::SIMDUtils::all;
    std::random_device rd;
    std::mt19937 mt (rd());
    std::uniform_real_distribution<float> dist (-1.0f, 1.0f);

    SECTION ("Data Test")
    {
        chowdsp::Buffer<TestType> buffer { 2, 128 };
        for (auto [_, x] : chowdsp::buffer_iters::channels (buffer))
        {
            for (int n = 0; n < buffer.getNumSamples(); ++n)
                x[n] = TestType (dist (mt));
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
            for (int n = 0; n < view.getNumSamples(); ++n)
                x[n] = TestType (dist (mt));
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
            for (int n = 0; n < view.getNumSamples(); ++n)
                xWrite[n] = TestType (dist (mt));
        }

        view.clear();
        for (const auto [_, xRead] : chowdsp::buffer_iters::channels (view))
        {
            for (int n = 0; n < view.getNumSamples(); ++n)
                REQUIRE_MESSAGE (all (xRead[n] == TestType (0)), "Data in view not correct!");
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
}
