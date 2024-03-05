#include <CatchUtils.h>
#include <chowdsp_filters/chowdsp_filters.h>

namespace Constants
{
constexpr float fs = 48000.0f;
constexpr float fc_vals[] = { 100.0f, 500.0f, 1500.0f, 3000.0f };
constexpr float Qval = 0.7071f;
constexpr float gainDB = 6.0f;
} // namespace Constants

template <typename T, typename FilterType, typename NumericType = chowdsp::SampleTypeHelpers::NumericType<T>>
void testFrequency (FilterType& filter, NumericType freq, NumericType expGainDB, NumericType maxError, const std::string& message)
{
    auto buffer = test_utils::makeSineWave<T> (freq, (NumericType) Constants::fs, (NumericType) 1);

    filter.reset();
    filter.processBlock (buffer);

    using namespace chowdsp::SIMDUtils;
    const auto halfSamples = buffer.getNumSamples() / 2;
    const auto magDB = gainToDecibels (chowdsp::BufferMath::getMagnitude (buffer, halfSamples, halfSamples));

    REQUIRE_MESSAGE (magDB == SIMDApprox<T> ((T) expGainDB).margin (maxError), message);
}

using CoeffMode = chowdsp::CoefficientCalculators::CoefficientCalculationMode;
TEMPLATE_TEST_CASE_SIG ("Second Order Filters Test", "[dsp][filters][simd]", ((typename T, CoeffMode Mode), T, Mode), (float, CoeffMode::Standard), (float, CoeffMode::Decramped), (double, CoeffMode::Standard), (double, CoeffMode::Decramped), (xsimd::batch<float>, CoeffMode::Standard), (xsimd::batch<float>, CoeffMode::Decramped), (xsimd::batch<double>, CoeffMode::Standard), (xsimd::batch<double>, CoeffMode::Decramped))
{
    using NumericType = chowdsp::SampleTypeHelpers::NumericType<T>;
    static constexpr auto maxError = (NumericType) 1.0e-2;

    SECTION ("Second Order LPF Test")
    {
        using namespace Constants;
        chowdsp::SecondOrderLPF<T, Mode> filter;

        for (auto fc : fc_vals)
        {
            filter.calcCoefs ((T) fc, (T) Qval, (NumericType) fs);
            testFrequency<T> (filter, (NumericType) 1, (NumericType) 0, maxError, "Incorrect gain at low frequencies. fc = " + std::to_string (fc));
            testFrequency<T> (filter, (NumericType) fc, (NumericType) -3.01, (NumericType) 0.2, "Incorrect gain at cutoff frequency. fc = " + std::to_string (fc));

            if (fc < 1000.0f || Mode == CoeffMode::Decramped)
            {
                const auto errorLimit = fc < 2000.0f ? (NumericType) 0.1 : (NumericType) 0.4;
                testFrequency<T> (filter, (NumericType) fc * (NumericType) 2, (NumericType) -12.3, errorLimit, "Incorrect gain at high frequencies. fc = " + std::to_string (fc));
                testFrequency<T> (filter, (NumericType) fc * (NumericType) 4, (NumericType) -24.2, errorLimit, "Incorrect gain at high frequencies. fc = " + std::to_string (fc));
            }
        }
    }

    SECTION ("Second Order HPF Test")
    {
        using namespace Constants;
        chowdsp::SecondOrderHPF<T, Mode> filter;

        for (auto fc : fc_vals)
        {
            filter.calcCoefs ((T) fc, (T) Qval, (NumericType) fs);
            testFrequency<T> (filter, (NumericType) fc / (NumericType) 4, (NumericType) -24.2, (NumericType) 0.4, "Incorrect gain at low frequencies. fc = " + std::to_string (fc));
            testFrequency<T> (filter, (NumericType) fc, (NumericType) -3.01, (NumericType) 4.0e-2, "Incorrect gain at cutoff frequency. fc = " + std::to_string (fc));
            testFrequency<T> (filter, (NumericType) fs * (NumericType) 0.498, (NumericType) 0, maxError, "Incorrect gain at high frequencies. fc = " + std::to_string (fc));
        }
    }

    SECTION ("Second Order BPF Test")
    {
        using namespace Constants;
        chowdsp::SecondOrderBPF<T, Mode, 2> filter;

        for (auto fc : fc_vals)
        {
            const auto errorLimitSkirt = (fc < 1000.0f || Mode == CoeffMode::Decramped) ? (NumericType) 0.1 : (NumericType) 1.0;
            const auto errorLimitCenter = (Mode == CoeffMode::Decramped) ? (NumericType) 0.1 : maxError;
            filter.calcCoefs ((T) fc, (T) Qval, (NumericType) fs);
            testFrequency<T> (filter, (NumericType) fc / (NumericType) 4, (NumericType) -9.0, errorLimitSkirt, "Incorrect gain at low frequencies. fc = " + std::to_string (fc));
            testFrequency<T> (filter, (NumericType) fc, (NumericType) 0, errorLimitCenter, "Incorrect gain at center frequency. fc = " + std::to_string (fc));
            if (fc < 2000.0f)
                testFrequency<T> (filter, (NumericType) fc * (NumericType) 4, (NumericType) -9.1, errorLimitSkirt, "Incorrect gain at high frequencies. fc = " + std::to_string (fc));
        }
    }

    SECTION ("Notch Filter Test")
    {
        using namespace Constants;
        chowdsp::NotchFilter<T, Mode> filter;

        for (auto fc : fc_vals)
        {
            filter.calcCoefs ((T) fc, (T) Qval, (NumericType) fs);

            testFrequency<T> (filter, (NumericType) 1, (NumericType) 0, maxError, "Incorrect gain at low frequencies. fc = " + std::to_string (fc));
            if (fc < 1000.0f)
            {
                if constexpr (std::is_same_v<NumericType, float>)
                    testFrequency<T> (filter, (NumericType) fc, (NumericType) -72, (NumericType) 5, "Incorrect gain at center frequency. fc = " + std::to_string (fc));
                else
                    testFrequency<T> (filter, (NumericType) fc, (NumericType) -100, (NumericType) 5, "Incorrect gain at center frequency. fc = " + std::to_string (fc));
            }
            if (fc < 2000.0f)
            {
                const auto errorLimit = (fc < 1000.0f && Mode == CoeffMode::Decramped) ? maxError : (NumericType) 0.1;
                testFrequency<T> (filter, (NumericType) fs * (NumericType) 0.498, (NumericType) 0, errorLimit, "Incorrect gain at high frequencies. fc = " + std::to_string (fc));
            }
        }
    }

    SECTION ("Peaking Filter Test")
    {
        using namespace Constants;
        chowdsp::PeakingFilter<T, Mode, 1> filter;

        for (auto fc : fc_vals)
        {
            filter.calcCoefsDB ((T) fc, (T) Qval, (T) gainDB, (NumericType) fs);
            testFrequency<T> (filter, (NumericType) 1, (NumericType) 0, maxError, "Incorrect gain at low frequencies. fc = " + std::to_string (fc));
            testFrequency<T> (filter, (NumericType) fc, (NumericType) gainDB, maxError, "Incorrect gain at center frequency. fc = " + std::to_string (fc));

            if (fc < 2000.0f || Mode == CoeffMode::Standard)
            {
                const auto errorLimit = fc < 1000.0f ? maxError : (NumericType) 0.4;
                testFrequency<T> (filter, (NumericType) fs * (NumericType) 0.498, (NumericType) 0, errorLimit, "Incorrect gain at high frequencies. fc = " + std::to_string (fc));
            }
        }
    }

    SECTION ("Low Shelf Filter Test")
    {
        using namespace Constants;
        chowdsp::LowShelfFilter<T, Mode> filter;

        for (auto fc : fc_vals)
        {
            filter.calcCoefsDB ((T) fc, (T) Qval, (T) gainDB, (NumericType) fs);
            testFrequency<T> (filter, (NumericType) 1, (NumericType) gainDB, maxError, "Incorrect gain at low frequencies. fc = " + std::to_string (fc));
            const auto errorLimitFc = (fc < 1000.0f && Mode == CoeffMode::Decramped) ? maxError : (NumericType) 0.1;
            testFrequency<T> (filter, (NumericType) fc, (NumericType) gainDB * (NumericType) 0.5, errorLimitFc, "Incorrect gain at cutoff frequency. fc = " + std::to_string (fc));
            testFrequency<T> (filter, (NumericType) fs * (NumericType) 0.498, (NumericType) 0, maxError, "Incorrect gain at high frequencies. fc = " + std::to_string (fc));
        }
    }

    SECTION ("High Shelf Filter Test")
    {
        using namespace Constants;
        chowdsp::HighShelfFilter<T, Mode> filter;

        for (auto fc : fc_vals)
        {
            filter.calcCoefsDB ((T) fc, (T) Qval, (T) gainDB, (NumericType) fs);
            const auto errorLimitLow = (fc > 200.0f) ? maxError : (NumericType) 0.025;
            testFrequency<T> (filter, (NumericType) 1, (NumericType) 0, errorLimitLow, "Incorrect gain at low frequencies. fc = " + std::to_string (fc));
            const auto errorLimitFc = (fc < 1000.0f && Mode == CoeffMode::Decramped) ? maxError : (NumericType) 0.1;
            testFrequency<T> (filter, (NumericType) fc, (NumericType) gainDB * (NumericType) 0.5, errorLimitFc, "Incorrect gain at cutoff frequency. fc = " + std::to_string (fc));
            testFrequency<T> (filter, (NumericType) fs * (NumericType) 0.498, (NumericType) gainDB, maxError, "Incorrect gain at high frequencies. fc = " + std::to_string (fc));
        }
    }
}
