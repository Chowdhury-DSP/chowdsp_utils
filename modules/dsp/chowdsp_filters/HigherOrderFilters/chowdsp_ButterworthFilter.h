#pragma once

namespace chowdsp
{
/** Filter type options for Butterworth Filters */
enum class ButterworthFilterType
{
    Lowpass,
    Highpass
};

/** A variable-order Butterworth filter */
template <int order, ButterworthFilterType type = ButterworthFilterType::Lowpass, typename FloatType = float, typename = void>
class ButterworthFilter;

/** A variable-order Butterworth filter (Even-Order specialization) */
template <int order, ButterworthFilterType type, typename FloatType>
class ButterworthFilter<order, type, FloatType, std::enable_if_t<order % 2 == 0>> : public SOSFilter<order, FloatType>
{
    static constexpr auto NFilters = (size_t) order / 2;

public:
    using NumericType = SampleTypeHelpers::NumericType<FloatType>;
    static constexpr bool HasQParameter = true;
    static constexpr bool HasGainParameter = false;

    ButterworthFilter() = default;

    /** Calculates the coefficients for a higher-order Butterworth filter */
    void calcCoefs (FloatType fc, FloatType qVal, NumericType fs)
    {
        calcCoefs (fc, qVal, fs, this->secondOrderSections, butterQVals);
    }

    /** Calculates the coefficients for a higher-order Butterworth filter */
    static void calcCoefs (FloatType fc, FloatType qVal, NumericType fs, std::array<IIRFilter<2, FloatType>, NFilters>& sections, const std::array<NumericType, NFilters>& butterQs)
    {
        FloatType bCoefs[3], aCoefs[3];
        auto calcCoefsForQ = [&] (FloatType stageQ, size_t stageOrder)
        {
            switch (type)
            {
                case ButterworthFilterType::Lowpass:
                    CoefficientCalculators::calcSecondOrderLPF (bCoefs, aCoefs, fc, stageQ, fs);
                    break;
                case ButterworthFilterType::Highpass:
                    CoefficientCalculators::calcSecondOrderHPF (bCoefs, aCoefs, fc, stageQ, fs);
                    break;
            }

            sections[stageOrder].setCoefs (bCoefs, aCoefs);
        };

        calcCoefsForQ (butterQs[0] * qVal * juce::MathConstants<NumericType>::sqrt2, 0);
        for (size_t i = 1; i < NFilters; ++i)
            calcCoefsForQ (butterQs[i], i);
    }

private:
    static constexpr std::array<NumericType, NFilters> butterQVals = QValCalcs::butterworth_Qs<NumericType, (size_t) order>();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ButterworthFilter)
};

/** A variable-order Butterworth filter (Odd-Order specialization) */
template <int order, ButterworthFilterType type, typename FloatType>
class ButterworthFilter<order, type, FloatType, std::enable_if_t<order % 2 == 1>> : public SOSFilter<order - 1, FloatType>
{
    static constexpr auto NFilters = (size_t) order / 2;

public:
    using NumericType = SampleTypeHelpers::NumericType<FloatType>;
    static constexpr bool HasQParameter = true;
    static constexpr bool HasGainParameter = false;
    static constexpr auto Order = order;

    ButterworthFilter() = default;

    /** Calculates the coefficients for a higher-order Butterworth filter */
    void calcCoefs (FloatType fc, FloatType qVal, NumericType fs)
    {
        ButterworthFilter<order - 1, type, FloatType>::calcCoefs (fc, qVal, fs, this->secondOrderSections, butterQVals);

        FloatType bCoefs1[2], aCoefs1[2];
        switch (type)
        {
            case ButterworthFilterType::Lowpass:
                CoefficientCalculators::calcFirstOrderLPF (bCoefs1, aCoefs1, fc, fs);
                break;
            case ButterworthFilterType::Highpass:
                CoefficientCalculators::calcFirstOrderHPF (bCoefs1, aCoefs1, fc, fs);
                break;
        }

        this->firstOrderSection.setCoefs (bCoefs1, aCoefs1);
    }

    /** Prepares the filter to process a new stream of audio */
    void prepare (int numChannels)
    {
        SOSFilter<order - 1, FloatType>::prepare (numChannels);
        firstOrderSection.prepare (numChannels);
    }

    /** Resets the filter state */
    void reset()
    {
        SOSFilter<order - 1, FloatType>::reset();
        firstOrderSection.reset();
    }

    /** Process a single sample */
    inline FloatType processSample (FloatType x, int channel = 0) noexcept
    {
        x = SOSFilter<order - 1, FloatType>::processSample (x, channel);
        x = firstOrderSection.processSample (x, channel);
        return x;
    }

    /** Process block of samples */
    void processBlock (FloatType* block, const int numSamples, const int channel = 0) noexcept
    {
        SOSFilter<order - 1, FloatType>::processBlock (block, numSamples, channel);
        firstOrderSection.processBlock (block, numSamples, channel);
    }

    /** Process block of samples */
    void processBlock (const BufferView<FloatType>& block) noexcept
    {
        SOSFilter<order - 1, FloatType>::processBlock (block);
        firstOrderSection.processBlock (block);
    }

    /** Process block of samples with a custom modulation callback which is called every sample */
    template <typename Modulator>
    void processBlockWithModulation (const BufferView<FloatType>& block, Modulator&& modulator) noexcept
    {
        const auto numChannels = block.getNumChannels();
        const auto numSamples = block.getNumSamples();

        auto* channelPointers = block.getArrayOfWritePointers();
        for (int n = 0; n < numSamples; ++n)
        {
            modulator (n);
            for (int channel = 0; channel < numChannels; ++channel)
                channelPointers[channel][n] = processSample (channelPointers[channel][n], channel);
        }
    }

private:
    IIRFilter<1, FloatType> firstOrderSection;

    static constexpr std::array<NumericType, NFilters> butterQVals = QValCalcs::butterworth_Qs<NumericType, (size_t) order>();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ButterworthFilter)
};
} // namespace chowdsp
