#pragma once

namespace chowdsp
{
/**
 * Utility class for downsampling a signal by an integer ratio.
 * Note that the downsampler must be used to process block sizes
 * that are an integer multiple of the downsampling ratio.
 *
 * @tparam T                        Data type to process
 * @tparam AntiAliasingFilterType   Filter type to use for an anti-aliasing filter, for example chowdsp::ButterworthFilter<8>
 */
template <typename T, typename AntiAliasingFilterType, bool allocateInternalBuffer = true>
class Downsampler
{
public:
    Downsampler() = default;

    /** Prepares the downsampler to process signal at a given downsampling ratio */
    void prepare (juce::dsp::ProcessSpec spec, int downsampleRatio)
    {
        ratio = downsampleRatio;
        if constexpr (allocateInternalBuffer)
            downsampledBuffer.setMaxSize ((int) spec.numChannels, (int) spec.maximumBlockSize / ratio);

        aaFilter.prepare ((int) spec.numChannels);

        auto fc = T (0.995 * ((spec.sampleRate / ratio) * 0.5));
        aaFilter.calcCoefs (fc, CoefficientCalculators::butterworthQ<T>, (T) spec.sampleRate);

        reset();
    }

    /** Resets the downsampler state */
    void reset()
    {
        aaFilter.reset();
        if constexpr (allocateInternalBuffer)
            downsampledBuffer.clear();
    }

    /** Returns the current downsampling ratio */
    [[nodiscard]] int getDownsamplingRatio() const noexcept { return ratio; }

    /**
     * Process a single-channel block of data
     * @param data              Block of input samples.
     * @param downsampledData   Block of output samples. Must point to a block of memory of size numSamples / ratio.
     * @param channel           Channel index to process.
     * @param numSamples        Number of input samples to process. Must be an integer multiple of the ratio.
     */
    void process (const T* data, T* downsampledData, const int channel, const int numSamples) noexcept
    {
        if (ratio == 1)
        {
            if (data == downsampledData)
                return;

            juce::FloatVectorOperations::copy (downsampledData, data, numSamples);
            return;
        }

        // Downsampler must be used on blocks with sizes that are integer multiples of the downsampling ratio!
        jassert (numSamples % ratio == 0);

#if ! JUCE_TEENSY
        if (numSamples <= 4096)
        {
            JUCE_BEGIN_IGNORE_WARNINGS_MSVC (6255 6386)
            auto filteredData = (T*) alloca ((size_t) numSamples * sizeof (T));
            aaFilter.processBlock (data, filteredData, numSamples, channel);
            for (int n = 0; n < numSamples / ratio; ++n)
                downsampledData[n] = filteredData[(n + 1) * ratio - 1];
            JUCE_END_IGNORE_WARNINGS_MSVC
        }
        else
#endif
        {
            for (int n = 0; n < numSamples / ratio; ++n)
            {
                for (int i = 0; i < ratio - 1; ++i)
                {
                    auto _ = aaFilter.processSample (data[n * ratio + i], channel);
                    juce::ignoreUnused (_);
                }
                downsampledData[n] = aaFilter.processSample (data[(n + 1) * ratio - 1], channel);
            }
        }
    }

    /**
     * Process a block of data.
     * Note that the block size must be an integer multiple of the downsampling ratio.
     */
    template <bool internalAlloc = allocateInternalBuffer>
    std::enable_if_t<internalAlloc, BufferView<T>>
        process (const BufferView<const T>& block) noexcept
    {
        process (block, downsampledBuffer);
        return { downsampledBuffer, 0, block.getNumSamples() / ratio, 0, block.getNumChannels() };
    }

    /**
     * Process a block of data, and stores the result in the given dsBuffer.
     * Note that the block size must be an integer multiple of the downsampling ratio.
     */
    void process (const BufferView<const T>& buffer, const BufferView<T>& dsBuffer) noexcept
    {
        const auto numChannels = buffer.getNumChannels();
        const auto numSamples = buffer.getNumSamples();

        // Downsampler must be used on blocks with sizes that are integer multiples of the downsampling ratio!
        jassert (numSamples % ratio == 0);

        for (int ch = 0; ch < numChannels; ++ch)
            process (buffer.getReadPointer (ch), dsBuffer.getWritePointer (ch), ch, numSamples);
    }

private:
    int ratio = 1;
    AntiAliasingFilterType aaFilter;

    std::conditional_t<allocateInternalBuffer, Buffer<T>, NullType> downsampledBuffer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Downsampler)
};

#if ! CHOWDSP_NO_XSIMD
/**
 * Utility class for downsampling a signal by an integer ratio,
 * using SIMD-accelerated anti-aliasing filters.
 * Note that the downsampler must be used to process block sizes
 * that are an integer multiple of the downsampling ratio.
 * Note that this implementation adds num_filters * (v_size - 1)
 * samples of latency on top of the non-SIMD
 * implementation's latency (at the higher sample rate).
 *
 * @tparam T                        Data type to process
 * @tparam AntiAliasingFilterType   Filter type to use for an anti-aliasing filter, for example chowdsp::ButterworthFilter<8>
 */
template <typename T, typename AntiAliasingFilterType, size_t maxNumChannels = defaultChannelCount>
class SIMDDownsampler
{
public:
    SIMDDownsampler() = default;

    /** Prepares the downsampler to process signal at a given downsampling ratio */
    void prepare (juce::dsp::ProcessSpec spec, int downsampleRatio)
    {
        ratio = downsampleRatio;

        AntiAliasingFilterType aaFilter;
        auto fc = T (0.995 * ((spec.sampleRate / ratio) * 0.5));
        aaFilter.calcCoefs (fc, CoefficientCalculators::butterworthQ<T>, (T) spec.sampleRate);

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
                a1[j] = aaFilter.secondOrderSections[i * v_size + j].a[1];
                a2[j] = aaFilter.secondOrderSections[i * v_size + j].a[2];
                b0[j] = aaFilter.secondOrderSections[i * v_size + j].b[0];
                b1[j] = aaFilter.secondOrderSections[i * v_size + j].b[1];
                b2[j] = aaFilter.secondOrderSections[i * v_size + j].b[2];
            }

            filter.secondOrderSections[i].a[1] = xsimd::load_aligned (a1.data());
            filter.secondOrderSections[i].a[2] = xsimd::load_aligned (a2.data());
            filter.secondOrderSections[i].b[0] = xsimd::load_aligned (b0.data());
            filter.secondOrderSections[i].b[1] = xsimd::load_aligned (b1.data());
            filter.secondOrderSections[i].b[2] = xsimd::load_aligned (b2.data());
        }

        reset();
    }

    /** Resets the downsampler state */
    void reset()
    {
        filter.reset();
        x_simd = {};
    }

    /** Returns the current downsampling ratio */
    [[nodiscard]] int getDownsamplingRatio() const noexcept { return ratio; }

    /**
     * Process a single-channel block of data
     * @param data              Block of input samples.
     * @param downsampledData   Block of output samples. Must point to a block of memory of size numSamples / ratio.
     * @param channel           Channel index to process.
     * @param numSamples        Number of input samples to process. Must be an integer multiple of the ratio.
     */
    void process (const T* data, T* downsampledData, const int channel, int numSamples) noexcept
    {
        if (ratio == 1)
        {
            if (data == downsampledData)
                return;

            juce::FloatVectorOperations::copy (downsampledData, data, numSamples);
            return;
        }

        // Downsampler must be used on blocks with sizes that are integer multiples of the downsampling ratio!
        jassert (numSamples % ratio == 0);

        while (numSamples > 0)
        {
            const auto N = std::min (numSamples, Math::round_to_next_multiple (2048, ratio));
            jassert (N % ratio == 0);

            JUCE_BEGIN_IGNORE_WARNINGS_MSVC (6255 6386)
            auto filteredData = (T*) alloca ((size_t) N * sizeof (T));
            processFilter (data, filteredData, channel, N);
            for (int n = 0; n < N / ratio; ++n)
                downsampledData[n] = filteredData[(n + 1) * ratio - 1];
            JUCE_END_IGNORE_WARNINGS_MSVC

            data += N;
            downsampledData += N / ratio;
            numSamples -= N;
        }
    }

    /**
     * Process a block of data, and stores the result in the given dsBuffer.
     * Note that the block size must be an integer multiple of the downsampling ratio.
     */
    void process (const BufferView<const T>& buffer, const BufferView<T>& dsBuffer) noexcept
    {
        const auto numChannels = buffer.getNumChannels();
        const auto numSamples = buffer.getNumSamples();

        // Downsampler must be used on blocks with sizes that are integer multiples of the downsampling ratio!
        jassert (numSamples % ratio == 0);

        for (int ch = 0; ch < numChannels; ++ch)
            process (buffer.getReadPointer (ch), dsBuffer.getWritePointer (ch), ch, numSamples);
    }

private:
    void processFilter (const T* data_in, T* data_out, const int channel, const int numSamples) noexcept
    {
        for (size_t i = 0; i < num_filters; ++i)
        {
            auto* cur_data_in = i == 0 ? data_in : data_out;
            auto& sos = filter.secondOrderSections[i];
            auto z1 = sos.z[(size_t) channel][1];
            auto z2 = sos.z[(size_t) channel][2];
            auto x_simd_ = x_simd[(size_t) channel][i];

            for (int n = 0; n < numSamples; ++n)
            {
                x_simd_[0] = cur_data_in[n];
                auto x = xsimd::load_aligned (x_simd_.data());

                x = sos.processSample2ndOrder (x, z1, z2);

                xsimd::store_aligned (x_simd_.data(), x);
                data_out[n] = x_simd_.back();
                for (size_t j = v_size - 1; j > 0; --j)
                    x_simd_[j] = x_simd_[j - 1];
            }

            sos.z[(size_t) channel][1] = z1;
            sos.z[(size_t) channel][2] = z2;
            x_simd[(size_t) channel][i] = x_simd_;
        }
    }

    int ratio = 1;

    static constexpr auto filter_order = AntiAliasingFilterType::Order;
    static constexpr auto v_size = xsimd::batch<T>::size;
    static_assert (filter_order % v_size == 0, "Filter order must be a multiple of the SIMD width on this platform!");
    static constexpr auto num_filters = filter_order / (2 * v_size);
    SOSFilter<filter_order / v_size, xsimd::batch<T>> filter;

    static_assert (maxNumChannels != dynamicChannelCount, "SIMD Upsampler cannot be used with a dynamic channel count!");
    alignas (16) std::array<std::array<std::array<T, v_size>, num_filters>, maxNumChannels> x_simd {};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SIMDDownsampler)
};
#endif
} // namespace chowdsp
