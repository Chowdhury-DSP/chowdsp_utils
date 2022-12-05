#pragma once

namespace chowdsp
{
/**
 * Copies data from a chowdsp::Buffer of some scalar type,
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
template <typename T>
[[maybe_unused]] static void copyToSIMDBuffer (const Buffer<T>& scalarBuffer, Buffer<xsimd::batch<T>>& simdBuffer) noexcept
{
    using Vec = xsimd::batch<T>;
    static constexpr auto vecSize = (int) Vec::size;

    const auto numSamples = scalarBuffer.getNumSamples();
    const auto numScalarChannels = scalarBuffer.getNumChannels();
    const auto numSIMDChannels = Math::ceiling_divide (numScalarChannels, vecSize);

    const auto interleaveSamples = [numSamples] (const T** source, T* dest, int numChannels) {
        std::fill (dest, dest + numSamples * vecSize, (T) 0);

        for (int chan = 0; chan < numChannels; ++chan)
        {
            auto i = chan;
            const auto* src = source[chan];

            for (int j = 0; j < numSamples; ++j)
            {
                dest[i] = src[j];
                i += vecSize;
            }
        }
    };

    simdBuffer.setCurrentSize (numSIMDChannels, numSamples);

    for (int ch = 0; ch < numScalarChannels; ch += vecSize)
    {
        const auto channelsToInterleave = juce::jmin (vecSize, numScalarChannels - ch);

        const T* scalarChannelPointers[(size_t) vecSize] {};
        for (int i = 0; i < channelsToInterleave; ++i)
        {
            const auto scalarChannelIndex = ch + i;
            scalarChannelPointers[i] = scalarBuffer.getReadPointer (scalarChannelIndex);
        }

        auto* simdChannelData = reinterpret_cast<T*> (simdBuffer.getWritePointer (ch / vecSize));
        interleaveSamples (scalarChannelPointers, simdChannelData, channelsToInterleave);
    }
}

/**
 * Copies data from a chowdsp::Buffer of some SIMD type,
 * to a chowdsp::Buffer of the corresponding scalar type.
 *
 * The scalar buffer will NOT be resized to match the size
 * of the SIMD buffer.
 */
template <typename T>
[[maybe_unused]] static void copyFromSIMDBuffer (const Buffer<xsimd::batch<T>>& simdBuffer, Buffer<T>& scalarBuffer) noexcept
{
    using Vec = xsimd::batch<T>;
    static constexpr auto vecSize = (int) Vec::size;

    const auto numSamples = simdBuffer.getNumSamples();
    const auto numSIMDChannels = simdBuffer.getNumChannels();

    jassert (scalarBuffer.getNumSamples() == numSamples); // Scalar buffer must have the same number of samples!
    jassert (scalarBuffer.getNumChannels() <= numSIMDChannels * vecSize); // Scalar buffer does not have enough channels!

    const auto deinterleaveSamples = [numSamples] (const T* source, T** dest, int numChannels) {
        for (int chan = 0; chan < numChannels; ++chan)
        {
            auto i = chan;
            auto* dst = dest[chan];

            for (int j = 0; j < numSamples; ++j)
            {
                dst[j] = source[i];
                i += vecSize;
            }
        }
    };

    for (int ch = 0; ch < numSIMDChannels; ++ch)
    {
        const auto channelsToDeinterleave = juce::jmin (vecSize, scalarBuffer.getNumChannels() - ch * vecSize);

        T* scalarChannelPointers[vecSize] {};
        for (int i = 0; i < channelsToDeinterleave; ++i)
        {
            const auto scalarChannelIndex = ch * vecSize + i;
            scalarChannelPointers[i] = scalarBuffer.getWritePointer (scalarChannelIndex);
        }

        auto* simdChannelData = reinterpret_cast<const T*> (simdBuffer.getReadPointer (ch));
        deinterleaveSamples (simdChannelData, scalarChannelPointers, channelsToDeinterleave);
    }
}
} // namespace chowdsp
