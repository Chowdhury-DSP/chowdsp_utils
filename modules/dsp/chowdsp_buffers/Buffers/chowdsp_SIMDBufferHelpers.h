#pragma once

namespace chowdsp
{
#if ! CHOWDSP_NO_XSIMD
/**
 * Copies data from a chowdsp::BufferView of some scalar type,
 * to a chowdsp::BufferView of the corresponding SIMD type.
 *
 * The SIMD BufferView is expected to be the correct size to
 * hold the SIMD data. If the number of channels in the scalar
 * buffer does not divide evenly into the SIMD register size,
 * the SIMD buffer will be padded with some extra channels,
 * containing zeros.
 */
template <typename T1, typename T2 = T1>
[[maybe_unused]] static void copyToSIMDBuffer (const BufferView<const T1>& scalarBuffer, const BufferView<xsimd::batch<T2>>& simdBuffer) noexcept
{
    using Vec = xsimd::batch<T2>;
    static constexpr auto vecSize = (int) Vec::size;

    const auto numSamples = scalarBuffer.getNumSamples();
    const auto numScalarChannels = scalarBuffer.getNumChannels();

    const auto interleaveSamples = [numSamples] (const T1** source, T2* dest, int numChannels)
    {
        std::fill (dest, dest + numSamples * vecSize, (T2) 0);

        for (int chan = 0; chan < numChannels; ++chan)
        {
            auto i = chan;
            const auto* src = source[chan];

            for (int j = 0; j < numSamples; ++j)
            {
                dest[i] = static_cast<T2> (src[j]);
                i += vecSize;
            }
        }
    };

    for (int ch = 0; ch < numScalarChannels; ch += vecSize)
    {
        const auto channelsToInterleave = juce::jmin (vecSize, numScalarChannels - ch);

        const T1* scalarChannelPointers[(size_t) vecSize] {};
        for (int i = 0; i < channelsToInterleave; ++i)
        {
            const auto scalarChannelIndex = ch + i;
            scalarChannelPointers[i] = scalarBuffer.getReadPointer (scalarChannelIndex);
        }

        auto* simdChannelData = reinterpret_cast<T2*> (simdBuffer.getWritePointer (ch / vecSize)); // NOSONAR (reinterpret_cast is safe here)
        interleaveSamples (scalarChannelPointers, simdChannelData, channelsToInterleave);
    }
}

/**
 * Copies data from a chowdsp::BufferView of some scalar type,
 * to a chowdsp::Buffer of the corresponding SIMD type.
 *
 * The SIMD buffer must have at least enough memory allocated
 * to store all of the data in the scalar buffer. The SIMD buffer
 * will be resized (without allocating) to match the size of
 * the scalar buffer. If the number of channels in the scalar
 * buffer does not divide evenly into the SIMD register size,
 * the SIMD buffer will be padded with some extra channels,
 * containing zeros.
 */
template <typename T1, typename T2 = T1>
[[maybe_unused]] static void copyToSIMDBuffer (const BufferView<const T1>& scalarBuffer, Buffer<xsimd::batch<T2>>& simdBuffer) noexcept
{
    using Vec = xsimd::batch<T2>;
    static constexpr auto vecSize = (int) Vec::size;

    const auto numSamples = scalarBuffer.getNumSamples();
    const auto numScalarChannels = scalarBuffer.getNumChannels();
    const auto numSIMDChannels = buffers_detail::ceiling_divide (numScalarChannels, vecSize);

    simdBuffer.setCurrentSize (numSIMDChannels, numSamples);

    copyToSIMDBuffer (scalarBuffer, BufferView { simdBuffer });
}

/**
 * Copies data from a chowdsp::Buffer of some scalar type,
 * to a chowdsp::Buffer of the corresponding SIMD type.
 */
template <typename T1, typename T2 = T1>
[[maybe_unused]] static void copyToSIMDBuffer (const Buffer<T1>& scalarBuffer, Buffer<xsimd::batch<T2>>& simdBuffer) noexcept
{
    copyToSIMDBuffer (static_cast<const BufferView<const T1>&> (scalarBuffer), simdBuffer);
}

/**
 * Copies data from a chowdsp::BufferView of some SIMD type,
 * to a chowdsp::BufferView of the corresponding scalar type.
 */
template <typename T1, typename T2 = T1>
[[maybe_unused]] static void copyFromSIMDBuffer (const BufferView<const xsimd::batch<T2>>& simdBuffer, const BufferView<T1>& scalarBuffer) noexcept
{
    using Vec = xsimd::batch<T2>;
    static constexpr auto vecSize = (int) Vec::size;

    const auto numSamples = simdBuffer.getNumSamples();
    const auto numSIMDChannels = simdBuffer.getNumChannels();

    jassert (scalarBuffer.getNumSamples() == numSamples); // Scalar buffer must have the same number of samples!
    jassert (scalarBuffer.getNumChannels() <= numSIMDChannels * vecSize); // Scalar buffer does not have enough channels!

    const auto deinterleaveSamples = [numSamples] (const T2* source, T1** dest, int numChannels)
    {
        for (int chan = 0; chan < numChannels; ++chan)
        {
            auto i = chan;
            auto* dst = dest[chan];

            for (int j = 0; j < numSamples; ++j)
            {
                dst[j] = static_cast<T1> (source[i]);
                i += vecSize;
            }
        }
    };

    for (int ch = 0; ch < numSIMDChannels; ++ch)
    {
        const auto channelsToDeinterleave = std::min (vecSize, scalarBuffer.getNumChannels() - ch * vecSize);

        T1* scalarChannelPointers[(size_t) vecSize] {};
        for (int i = 0; i < channelsToDeinterleave; ++i)
        {
            const auto scalarChannelIndex = ch * vecSize + i;
            scalarChannelPointers[i] = scalarBuffer.getWritePointer (scalarChannelIndex);
        }

        auto* simdChannelData = reinterpret_cast<const T2*> (simdBuffer.getReadPointer (ch)); // NOSONAR (reinterpret_cast is safe here)
        deinterleaveSamples (simdChannelData, scalarChannelPointers, channelsToDeinterleave);
    }
}

/**
 * Copies data from a chowdsp::Buffer of some SIMD type,
 * to a chowdsp::Buffer of the corresponding scalar type.
 */
template <typename T1, typename T2 = T1>
[[maybe_unused]] static void copyFromSIMDBuffer (const Buffer<xsimd::batch<T2>>& simdBuffer, Buffer<T1>& scalarBuffer) noexcept
{
    copyFromSIMDBuffer<T1, T2> (simdBuffer, static_cast<const BufferView<T1>&> (scalarBuffer));
}
#endif // ! CHOWDSP_NO_XSIMD
} // namespace chowdsp
