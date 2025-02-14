#pragma once

namespace chowdsp
{
/**
 * Utility class for upsampling a signal by an integer ratio
 *
 * @tparam T                        Data type to process
 * @tparam AntiImagingFilterType    Filter type to use for an anti-imaging filter, for example chowdsp::ButterworthFilter<8>
 */
template <typename T, typename AntiImagingFilterType, bool allocateInternalBuffer = true>
class Upsampler
{
public:
    Upsampler() = default;

    /** Prepares the upsampler to process signal at a given upsampling ratio */
    void prepare (juce::dsp::ProcessSpec spec, int upsampleRatio)
    {
        ratio = upsampleRatio;
        if constexpr (allocateInternalBuffer)
            upsampledBuffer.setMaxSize ((int) spec.numChannels, (int) spec.maximumBlockSize * ratio);

        aiFilter.prepare ((int) spec.numChannels);

        auto fc = T (0.995 * (spec.sampleRate * 0.5));
        aiFilter.calcCoefs (fc, CoefficientCalculators::butterworthQ<T>, ratio * (T) spec.sampleRate);

        reset();
    }

    /** Resets the upsampler state */
    void reset()
    {
        aiFilter.reset();
        if constexpr (allocateInternalBuffer)
            upsampledBuffer.clear();
    }

    /** Returns the current upsampling ratio */
    [[nodiscard]] int getUpsamplingRatio() const noexcept { return ratio; }

    /**
     * Process a single-channel block of data
     * @param data              Block of input samples
     * @param upsampledData     Block of output samples. Must point to a block of memory of size numSamples * ratio
     * @param channel           Channel index to process
     * @param numSamples        Number of input samples to process
     */
    void process (const T* data, T* upsampledData, const int channel, const int numSamples) noexcept
    {
        if (ratio == 1)
        {
            if (data == upsampledData)
                return;

            juce::FloatVectorOperations::copy (upsampledData, data, numSamples);
            return;
        }

        for (int n = 0; n < numSamples; ++n)
        {
            int startSample = n * ratio;
            upsampledData[startSample] = data[n];

            for (int i = startSample + 1; i < startSample + ratio; ++i)
                upsampledData[i] = (T) 0;
        }

        aiFilter.processBlock (upsampledData, numSamples * ratio, channel);
        juce::FloatVectorOperations::multiply (upsampledData, (T) ratio, numSamples * ratio);
    }

    /** Process a block of data */
    template <bool internalAlloc = allocateInternalBuffer>
    std::enable_if_t<internalAlloc, BufferView<T>>
        process (const BufferView<const T>& block) noexcept
    {
        process (block, upsampledBuffer);
        return { upsampledBuffer, 0, block.getNumSamples() * ratio, 0, block.getNumChannels() };
    }

    /** Process a block of data, and stores the result in the given dsBuffer. */
    void process (const BufferView<const T>& buffer, const BufferView<T>& usBuffer) noexcept
    {
        const auto numChannels = buffer.getNumChannels();
        const auto numSamples = buffer.getNumSamples();

        for (int ch = 0; ch < numChannels; ++ch)
            process (buffer.getReadPointer (ch), usBuffer.getWritePointer (ch), ch, numSamples);
    }

private:
    int ratio = 1;
    AntiImagingFilterType aiFilter; // anti-imaging filter

    std::conditional_t<allocateInternalBuffer, Buffer<T>, NullType> upsampledBuffer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Upsampler)
};

#if ! CHOWDSP_NO_XSIMD
/**
 * Utility class for upsampling a signal by an integer ratio,
 * using SIMD-accelerated anti-imaging filters.
 * Note that this implementation adds num_filters * (v_size - 1)
 * samples of latency on top of the non-SIMD
 * implementation's latency (at the higher sample rate).
 *
 * @tparam T                        Data type to process
 * @tparam AntiImagingFilterType    Filter type to use for an anti-imaging filter, for example chowdsp::ButterworthFilter<8>
 */
template <typename T, typename AntiImagingFilterType, size_t maxNumChannels = defaultChannelCount>
class SIMDUpsampler
{
public:
    static constexpr auto filter_order = AntiImagingFilterType::Order;
    static constexpr auto v_size = xsimd::batch<T>::size;
    static_assert (filter_order % v_size == 0, "Filter order must be a multiple of the SIMD width on this platform!");
    static constexpr auto num_filters = filter_order / (2 * v_size);

    SIMDUpsampler() = default;

    /** Prepares the upsampler to process signal at a given upsampling ratio */
    void prepare (juce::dsp::ProcessSpec spec, int upsampleRatio)
    {
        ratio = upsampleRatio;

        AntiImagingFilterType aiFilter; // anti-imaging filter
        auto fc = T (0.995 * (spec.sampleRate * 0.5));
        aiFilter.calcCoefs (fc, CoefficientCalculators::butterworthQ<T>, ratio * (T) spec.sampleRate);

        filter.prepare ((int) spec.numChannels);
        alignas (SIMDUtils::defaultSIMDAlignment) std::array<T, v_size> a1 {};
        alignas (SIMDUtils::defaultSIMDAlignment) std::array<T, v_size> a2 {};
        alignas (SIMDUtils::defaultSIMDAlignment) std::array<T, v_size> b0 {};
        alignas (SIMDUtils::defaultSIMDAlignment) std::array<T, v_size> b1 {};
        alignas (SIMDUtils::defaultSIMDAlignment) std::array<T, v_size> b2 {};
        for (size_t i = 0; i < num_filters; ++i)
        {
            for (size_t j = 0; j < v_size; ++j)
            {
                a1[j] = aiFilter.secondOrderSections[i * v_size + j].a[1];
                a2[j] = aiFilter.secondOrderSections[i * v_size + j].a[2];
                b0[j] = aiFilter.secondOrderSections[i * v_size + j].b[0];
                b1[j] = aiFilter.secondOrderSections[i * v_size + j].b[1];
                b2[j] = aiFilter.secondOrderSections[i * v_size + j].b[2];
            }

            filter.secondOrderSections[i].a[1] = xsimd::load_aligned (a1.data());
            filter.secondOrderSections[i].a[2] = xsimd::load_aligned (a2.data());
            filter.secondOrderSections[i].b[0] = xsimd::load_aligned (b0.data());
            filter.secondOrderSections[i].b[1] = xsimd::load_aligned (b1.data());
            filter.secondOrderSections[i].b[2] = xsimd::load_aligned (b2.data());
        }

        reset();
    }

    /** Resets the upsampler state */
    void reset()
    {
        filter.reset();
        x_simd = {};
    }

    /** Returns the current upsampling ratio */
    [[nodiscard]] int getUpsamplingRatio() const noexcept { return ratio; }

    /**
     * Process a single-channel block of data
     * @param data              Block of input samples
     * @param upsampledData     Block of output samples. Must point to a block of memory of size numSamples * ratio
     * @param channel           Channel index to process
     * @param numSamples        Number of input samples to process
     */
    void process (const T* data, T* upsampledData, const int channel, const int numSamples) noexcept
    {
        if (ratio == 1)
        {
            if (data == upsampledData)
                return;

            juce::FloatVectorOperations::copy (upsampledData, data, numSamples);
            return;
        }

        for (int n = 0; n < numSamples; ++n)
        {
            int startSample = n * ratio;
            upsampledData[startSample] = data[n];

            for (int i = startSample + 1; i < startSample + ratio; ++i)
                upsampledData[i] = (T) 0;
        }

        processFilter (upsampledData, channel, numSamples * ratio);
        juce::FloatVectorOperations::multiply (upsampledData, (T) ratio, numSamples * ratio);
    }

    /** Process a block of data, and stores the result in the given dsBuffer. */
    void process (const BufferView<const T>& buffer, const BufferView<T>& usBuffer) noexcept
    {
        const auto numChannels = buffer.getNumChannels();
        const auto numSamples = buffer.getNumSamples();

        for (int ch = 0; ch < numChannels; ++ch)
            process (buffer.getReadPointer (ch), usBuffer.getWritePointer (ch), ch, numSamples);
    }

private:
    void processFilter (T* data, const int channel, const int numSamples) noexcept
    {
        for (size_t i = 0; i < num_filters; ++i)
        {
            auto& sos = filter.secondOrderSections[i];
            auto z1 = sos.z[(size_t) channel][1];
            auto z2 = sos.z[(size_t) channel][2];
            auto x_simd_ = x_simd[(size_t) channel][i];

            for (int n = 0; n < numSamples; ++n)
            {
                x_simd_[0] = data[n];
                auto x = xsimd::load_aligned (x_simd_.data());

                x = sos.processSample2ndOrder (x, z1, z2);

                xsimd::store_aligned (x_simd_.data(), x);
                data[n] = x_simd_.back();
                for (size_t j = v_size - 1; j > 0; --j)
                    x_simd_[j] = x_simd_[j - 1];
            }

            sos.z[(size_t) channel][1] = z1;
            sos.z[(size_t) channel][2] = z2;
            x_simd[(size_t) channel][i] = x_simd_;
        }
    }

    int ratio = 1;

    SOSFilter<filter_order / v_size, xsimd::batch<T>> filter;

    static_assert (maxNumChannels != dynamicChannelCount, "SIMD Upsampler cannot be used with a dynamic channel count!");
    alignas (16) std::array<std::array<std::array<T, v_size>, num_filters>, maxNumChannels> x_simd {};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SIMDUpsampler)
};
#endif
} // namespace chowdsp
