#pragma once

namespace chowdsp
{
#ifndef DOXYGEN
namespace detail
{
    template <typename BufferType>
    struct BufferInfoBase
    {
        using sample_type = std::remove_const_t<typename BufferType::Type>;
        using const_type = const BufferType;
        static constexpr bool is_buffer = true;
    };

    template <typename BufferType>
    struct BufferInfo
    {
        static constexpr bool is_buffer = false;
    };

    template <typename SampleType>
    struct BufferInfo<Buffer<SampleType>> : BufferInfoBase<Buffer<SampleType>>
    {
    };

    template <typename SampleType>
    struct BufferInfo<const Buffer<SampleType>> : BufferInfoBase<Buffer<SampleType>>
    {
    };

    template <typename SampleType, int nChannels, int nSamples>
    struct BufferInfo<StaticBuffer<SampleType, nChannels, nSamples>> : BufferInfoBase<StaticBuffer<SampleType, nChannels, nSamples>>
    {
    };

    template <typename SampleType, int nChannels, int nSamples>
    struct BufferInfo<const StaticBuffer<SampleType, nChannels, nSamples>> : BufferInfoBase<StaticBuffer<SampleType, nChannels, nSamples>>
    {
    };

    template <typename SampleType>
    struct BufferInfo<BufferView<SampleType>> : BufferInfoBase<BufferView<SampleType>>
    {
        using const_type = BufferView<const std::remove_const_t<SampleType>>;
    };

    template <typename SampleType>
    struct BufferInfo<const BufferView<SampleType>> : BufferInfoBase<BufferView<SampleType>>
    {
        using const_type = const BufferView<const std::remove_const_t<SampleType>>;
    };

#if CHOWDSP_USING_JUCE
    template <typename SampleType>
    struct BufferInfo<juce::AudioBuffer<SampleType>>
    {
        using sample_type = SampleType;
        using const_type = const juce::AudioBuffer<SampleType>;
        static constexpr bool is_buffer = true;
    };

    template <typename SampleType>
    struct BufferInfo<const juce::AudioBuffer<SampleType>>
    {
        using sample_type = SampleType;
        using const_type = const juce::AudioBuffer<SampleType>;
        static constexpr bool is_buffer = true;
    };
#endif

    static_assert (BufferInfo<std::string>::is_buffer == false);
    static_assert (BufferInfo<Buffer<float>>::is_buffer == true);
    static_assert (BufferInfo<StaticBuffer<float, 1, 10>>::is_buffer == true);
    static_assert (BufferInfo<BufferView<float>>::is_buffer == true);

    static_assert (std::is_same_v<BufferInfo<Buffer<float>>::sample_type, float>);
    static_assert (std::is_same_v<BufferInfo<Buffer<double>>::sample_type, double>);
    static_assert (std::is_same_v<BufferInfo<Buffer<float>>::const_type, const Buffer<float>>);
    static_assert (std::is_same_v<BufferInfo<const Buffer<float>>::const_type, const Buffer<float>>);
#if ! CHOWDSP_NO_XSIMD
    static_assert (std::is_same_v<BufferInfo<Buffer<xsimd::batch<float>>>::sample_type, xsimd::batch<float>>);
    static_assert (std::is_same_v<BufferInfo<Buffer<xsimd::batch<double>>>::sample_type, xsimd::batch<double>>);
#endif

    static_assert (std::is_same_v<BufferInfo<StaticBuffer<float, 1, 10>>::sample_type, float>);
    static_assert (std::is_same_v<BufferInfo<StaticBuffer<double, 1, 10>>::sample_type, double>);
    static_assert (std::is_same_v<BufferInfo<StaticBuffer<float, 1, 10>>::const_type, const StaticBuffer<float, 1, 10>>);
    static_assert (std::is_same_v<BufferInfo<const StaticBuffer<float, 1, 10>>::const_type, const StaticBuffer<float, 1, 10>>);
#if ! CHOWDSP_NO_XSIMD
    static_assert (std::is_same_v<BufferInfo<StaticBuffer<xsimd::batch<float>, 1, 10>>::sample_type, xsimd::batch<float>>);
    static_assert (std::is_same_v<BufferInfo<StaticBuffer<xsimd::batch<double>, 1, 10>>::sample_type, xsimd::batch<double>>);
#endif

    static_assert (std::is_same_v<BufferInfo<BufferView<float>>::sample_type, float>);
    static_assert (std::is_same_v<BufferInfo<BufferView<double>>::sample_type, double>);
    static_assert (std::is_same_v<BufferInfo<BufferView<float>>::const_type, BufferView<const float>>);
    static_assert (std::is_same_v<BufferInfo<const BufferView<float>>::const_type, const BufferView<const float>>);
    static_assert (std::is_same_v<BufferInfo<BufferView<const float>>::const_type, BufferView<const float>>);
    static_assert (std::is_same_v<BufferInfo<const BufferView<const float>>::const_type, const BufferView<const float>>);
#if ! CHOWDSP_NO_XSIMD
    static_assert (std::is_same_v<BufferInfo<Buffer<xsimd::batch<float>>>::sample_type, xsimd::batch<float>>);
    static_assert (std::is_same_v<BufferInfo<Buffer<xsimd::batch<double>>>::sample_type, xsimd::batch<double>>);
#endif

#if CHOWDSP_USING_JUCE
    static_assert (std::is_same_v<BufferInfo<juce::AudioBuffer<float>>::sample_type, float>);
    static_assert (std::is_same_v<BufferInfo<juce::AudioBuffer<double>>::sample_type, double>);
    static_assert (std::is_same_v<BufferInfo<juce::AudioBuffer<float>>::const_type, const juce::AudioBuffer<float>>);
    static_assert (std::is_same_v<BufferInfo<const juce::AudioBuffer<float>>::const_type, const juce::AudioBuffer<float>>);
#endif
} // namespace detail
#endif // DOXYGEN

/** Template helper for getting the associated const buffer type */
template <typename BufferType>
using ConstBufferType = typename detail::BufferInfo<std::remove_reference_t<BufferType>>::const_type;

/** Returns true if this buffer contains const data. */
template <typename BufferType>
static constexpr bool IsConstBuffer = std::is_same_v<ConstBufferType<BufferType>, std::remove_reference_t<BufferType>>;

/** Template helper for getting the sample type from a buffer. */
template <typename BufferType>
using BufferSampleType = typename detail::BufferInfo<BufferType>::sample_type;
} // namespace chowdsp
