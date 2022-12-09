#pragma once

namespace chowdsp
{
/**
 * An approximate fractional-order filter based on the
 * derivation provided in: https://www.dafx.de/paper-archive/2021/proceedings/papers/DAFx20in21_paper_18.pdf
 * @tparam FloatType    The floating-point type to use
 * @tparam N            The approximation order to use
 */
template <typename FloatType, int N = 4>
class FractionalOrderFilter
{
public:
    using NumericType = SampleTypeHelpers::NumericType<FloatType>;

    FractionalOrderFilter() = default;

    /** Prepares the filter to process a new stream of audio */
    void prepare (double sampleRate, int numChannels)
    {
        fRef = FloatType (sampleRate / 2.0);

        for (auto& filt : filters)
            filt.prepare (numChannels);
    }

    /** Resets the filter state */
    void reset()
    {
        for (auto& filt : filters)
            filt.reset();
    }

    /**
     * Calculates the filter coefficients.
     *
     * @param fc    The filter cutoff frequency
     * @param alpha A parameter between 0-1 controlling the filter rolloff slope between 0-3dB / octave
     * @param fs    The current audio sample rate
     */
    void calcCoefs (FloatType fc, FloatType alpha, NumericType fs)
    {
        const auto k = ConformalMaps::computeKValue (fc, fs);
        auto setFilterCoefs = [k] (auto& filter, FloatType pole, FloatType zero)
        {
            FloatType a_z[2] {};
            FloatType b_z[2] {};

            FloatType a_s[2] = { FloatType (1) / pole, FloatType (-1) };
            FloatType b_s[2] = { FloatType (1) / zero, FloatType (-1) };
            ConformalMaps::Transform<FloatType, 1>::bilinear (b_z, a_z, b_s, a_s, k);

            filter.setCoefs (b_z, a_z);
        };

        for (size_t i = 1; i <= (size_t) N; ++i)
        {
            const auto pole = -juce::MathConstants<NumericType>::twoPi * fc * std::pow (fRef / fc, (NumericType (2 * i - 1) - alpha) / (NumericType (2 * N + 1) - alpha));
            const auto zero = -juce::MathConstants<NumericType>::twoPi * fc * std::pow (fRef / fc, (NumericType (2 * i - 1) + alpha) / (NumericType (2 * N + 1) - alpha));
            setFilterCoefs (filters[i - 1], pole, zero);
        }
    }

    /** Process a single sample */
    inline FloatType processSample (FloatType x, int channel = 0) noexcept
    {
        for (auto& filt : filters)
            x = filt.processSample (x, channel);
        return x;
    }

    /** Process block of samples */
    void processBlock (FloatType* block, const int numSamples, const int channel = 0) noexcept
    {
        for (auto& filt : filters)
            filt.processBlock (block, numSamples, channel);
    }

    /** Process block of samples */
    void processBlock (const BufferView<FloatType>& block) noexcept
    {
        for (auto& filt : filters)
            filt.processBlock (block);
    }

private:
    std::array<IIRFilter<1, FloatType>, (size_t) N> filters;

    FloatType fRef {};

    std::vector<FloatType*> channelPointers = std::vector<FloatType*> (1);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FractionalOrderFilter)
};
} // namespace chowdsp
