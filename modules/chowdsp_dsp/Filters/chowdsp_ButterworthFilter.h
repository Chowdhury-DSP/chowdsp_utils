#pragma once

namespace chowdsp
{
/** Filter type options for Butterworth Filters */
enum class ButterworthFilterType
{
    Lowpass,
    Bandpass,
    Bandstop,
    Highpass
};

template <int order, ButterworthFilterType type = ButterworthFilterType::Lowpass, typename FloatType = float>
class ButterworthFilter : public SOSFilter<order, FloatType>
{
public:
    using NumericType = SampleTypeHelpers::NumericType<FloatType>;
    static constexpr bool HasQParameter = true;
    static constexpr bool HasGainParameter = false;

    ButterworthFilter() = default;

    void calcCoefs (FloatType fc, FloatType qVal, NumericType fs)
    {
        FloatType bCoefs[3], aCoefs[3];
        auto calcCoefsForQ = [&] (FloatType stageQ, size_t stageOrder)
        {
            switch (type)
            {
                case ButterworthFilterType::Lowpass:
                    CoefficientCalculators::calcSecondOrderLPF (bCoefs, aCoefs, fc, stageQ, fs);
                    break;
                case ButterworthFilterType::Bandpass:
                    CoefficientCalculators::calcSecondOrderBPF (bCoefs, aCoefs, fc, stageQ, fs);
                    break;
                case ButterworthFilterType::Bandstop:
                    CoefficientCalculators::calcNotchFilter (bCoefs, aCoefs, fc, stageQ, fs);
                    break;
                case ButterworthFilterType::Highpass:
                    CoefficientCalculators::calcSecondOrderHPF (bCoefs, aCoefs, fc, stageQ, fs);
                    break;
            }

            this->secondOrderSections[stageOrder].setCoefs (bCoefs, aCoefs);
        };

        calcCoefsForQ (butterQVals[0] * qVal * juce::MathConstants<NumericType>::sqrt2, 0);
        for (size_t i = 1; i < (size_t) order / 2; ++i)
            calcCoefsForQ (butterQVals[i], i);
    }

private:
    const std::array<NumericType, order / 2> butterQVals = QValCalcs::butterworth_Qs<FloatType, order>();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ButterworthFilter)
};
} // namespace chowdsp
