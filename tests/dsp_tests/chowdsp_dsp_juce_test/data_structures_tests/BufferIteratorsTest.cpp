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
                    (chowdsp::StaticBuffer<float, 2, 8>),
                    (chowdsp::StaticBuffer<double, 2, 8>),
                    (chowdsp::StaticBuffer<xsimd::batch<float>, 2, 8>),
                    (chowdsp::StaticBuffer<xsimd::batch<double>, 2, 8>),
                    (juce::AudioBuffer<float>),
                    (juce::AudioBuffer<double>) )
{
    using BufferType = TestType;
    using SampleType = chowdsp::BufferMath::detail::BufferSampleType<BufferType>;

    BufferType buffer { 2, 4 };

    {
        int count = 0;
        for (auto [channel, data] : chowdsp::buffer_iters::channels (buffer))
        {
            for (int i = 0; i < buffer.getNumSamples(); ++i)
                data[i] = (SampleType) static_cast<float> (count++);
        }
    }

    int count = 0;
    for (auto [channel, data] : chowdsp::buffer_iters::channels (chowdsp::asConstBuffer (buffer)))
    {
        for (int i = 0; i < buffer.getNumSamples(); ++i)
            REQUIRE (chowdsp::SIMDUtils::all ((SampleType) static_cast<float> (count++) == data[i]));
    }
}
