#include <CatchUtils.h>
#include <chowdsp_filters/chowdsp_filters.h>

namespace Constants
{
constexpr float fs = 48000.0f;
constexpr float fc = 500.0f;
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

TEMPLATE_TEST_CASE ("Second Order Filters Test", "", float, double, xsimd::batch<float>, xsimd::batch<double>)
{
    using T = TestType;
    using NumericType = chowdsp::SampleTypeHelpers::NumericType<T>;
    static constexpr auto maxError = (NumericType) 1.0e-2;

    SECTION ("Second Order LPF Test")
    {
        using namespace Constants;
        chowdsp::SecondOrderLPF<T> filter;
        filter.calcCoefs ((T) fc, (T) Qval, (NumericType) fs);

        testFrequency<T> (filter, (NumericType) 1, (NumericType) 0, maxError, "Incorrect gain at low frequencies.");
        testFrequency<T> (filter, (NumericType) fc, (NumericType) -3.01, maxError, "Incorrect gain at cutoff frequency.");
        testFrequency<T> (filter, (NumericType) fc * (NumericType) 4, (NumericType) -24.2, (NumericType) 0.1, "Incorrect gain at high frequencies.");
    }

    SECTION ("Second Order HPF Test")
    {
        using namespace Constants;
        chowdsp::SecondOrderHPF<T> filter;
        filter.calcCoefs ((T) fc, (T) Qval, (NumericType) fs);

        testFrequency<T> (filter, (NumericType) fc / (NumericType) 4, (NumericType) -24.1, (NumericType) 0.1, "Incorrect gain at low frequencies.");
        testFrequency<T> (filter, (NumericType) fc, (NumericType) -3.01, maxError, "Incorrect gain at cutoff frequency.");
        testFrequency<T> (filter, (NumericType) fs * (NumericType) 0.498, (NumericType) 0, maxError, "Incorrect gain at high frequencies.");
    }

    SECTION ("Second Order BPF Test")
    {
        using namespace Constants;
        chowdsp::SecondOrderBPF<T> filter;
        filter.calcCoefs ((T) fc, (T) Qval, (NumericType) fs);

        testFrequency<T> (filter, (NumericType) fc / (NumericType) 4, (NumericType) -9.0, (NumericType) 0.1, "Incorrect gain at low frequencies.");
        testFrequency<T> (filter, (NumericType) fc, (NumericType) 0, maxError, "Incorrect gain at center frequency.");
        testFrequency<T> (filter, (NumericType) fc * (NumericType) 4, (NumericType) -9.1, (NumericType) 0.1, "Incorrect gain at high frequencies.");
    }

    SECTION ("Notch Filter Test")
    {
        using namespace Constants;
        chowdsp::NotchFilter<T> filter;
        filter.calcCoefs ((T) fc, (T) Qval, (NumericType) fs);

        testFrequency<T> (filter, (NumericType) 1, (NumericType) 0, maxError, "Incorrect gain at low frequencies.");
        if constexpr (std::is_same_v<NumericType, float>)
            testFrequency<T> (filter, (NumericType) fc, (NumericType) -72, (NumericType) 5, "Incorrect gain at center frequency.");
        else
            testFrequency<T> (filter, (NumericType) fc, (NumericType) -100, (NumericType) 5, "Incorrect gain at center frequency.");
        testFrequency<T> (filter, (NumericType) fs * (NumericType) 0.498, (NumericType) 0, maxError, "Incorrect gain at high frequencies.");
    }

    SECTION ("Peaking Filter Test")
    {
        using namespace Constants;
        chowdsp::PeakingFilter<T> filter;
        filter.calcCoefsDB ((T) fc, (T) Qval, (T) gainDB, (NumericType) fs);

        testFrequency<T> (filter, (NumericType) 1, (NumericType) 0, maxError, "Incorrect gain at low frequencies.");
        testFrequency<T> (filter, (NumericType) fc, (NumericType) gainDB, maxError, "Incorrect gain at center frequency.");
        testFrequency<T> (filter, (NumericType) fs * (NumericType) 0.498, (NumericType) 0, maxError, "Incorrect gain at high frequencies.");
    }

    SECTION ("Low Shelf Filter Test")
    {
        using namespace Constants;
        chowdsp::LowShelfFilter<T> filter;
        filter.calcCoefsDB ((T) fc, (T) Qval, (T) gainDB, (NumericType) fs);

        testFrequency<T> (filter, (NumericType) 1, (NumericType) gainDB, maxError, "Incorrect gain at low frequencies.");
        testFrequency<T> (filter, (NumericType) fc, (NumericType) gainDB * (NumericType) 0.5, maxError, "Incorrect gain at cutoff frequency.");
        testFrequency<T> (filter, (NumericType) fs * (NumericType) 0.498, (NumericType) 0, maxError, "Incorrect gain at high frequencies.");
    }

    SECTION ("High Shelf Filter Test")
    {
        using namespace Constants;
        chowdsp::HighShelfFilter<T> filter;
        filter.calcCoefsDB ((T) fc, (T) Qval, (T) gainDB, (NumericType) fs);

        testFrequency<T> (filter, (NumericType) 1, (NumericType) 0, maxError, "Incorrect gain at low frequencies.");
        testFrequency<T> (filter, (NumericType) fc, (NumericType) gainDB * (NumericType) 0.5, maxError, "Incorrect gain at cutoff frequency.");
        testFrequency<T> (filter, (NumericType) fs * (NumericType) 0.498, (NumericType) gainDB, maxError, "Incorrect gain at high frequencies.");
    }
}
