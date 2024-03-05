#include <CatchUtils.h>
#include <chowdsp_filters/chowdsp_filters.h>

namespace Constants
{
constexpr float fs = 48000.0f;
constexpr float fc = 500.0f;
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

TEMPLATE_TEST_CASE ("First Order Filters Test", "[dsp][filters][simd]", float, double, xsimd::batch<float>, xsimd::batch<double>)
{
    using T = TestType;
    using NumericType = chowdsp::SampleTypeHelpers::NumericType<T>;
    static constexpr auto maxError = (NumericType) 1.0e-2;

    SECTION ("First Order LPF Test")
    {
        chowdsp::FirstOrderLPF<T> lpFilter;
        lpFilter.prepare (1);
        lpFilter.calcCoefs ((T) Constants::fc, (NumericType) Constants::fs);

        testFrequency<T> (lpFilter, (NumericType) 1, (NumericType) 0, maxError, "Incorrect gain at low frequencies.");
        testFrequency<T> (lpFilter, (NumericType) Constants::fc, (NumericType) -3.01, maxError, "Incorrect gain at cutoff frequency.");
        testFrequency<T> (lpFilter, Constants::fc * (NumericType) 4, (NumericType) -12.3, (NumericType) 0.1, "Incorrect gain at high frequencies.");
    }

    SECTION ("First Order HPF Test")
    {
        chowdsp::FirstOrderHPF<T, 1> hpFilter;
        hpFilter.prepare ({ Constants::fs, 128, 1 });
        hpFilter.calcCoefs ((T) Constants::fc, (NumericType) Constants::fs);

        testFrequency<T> (hpFilter, (NumericType) Constants::fc / 4, (NumericType) -12.3, (NumericType) 0.1, "Incorrect gain at low frequencies.");
        testFrequency<T> (hpFilter, (NumericType) Constants::fc, (NumericType) -3.01, maxError, "Incorrect gain at cutoff frequency.");
        testFrequency<T> (hpFilter, (NumericType) Constants::fs * (NumericType) 0.498, (NumericType) 0, maxError, "Incorrect gain at high frequencies.");
    }
}
