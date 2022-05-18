#include <test_utils.h>
#include <TimedUnitTest.h>
#include <chowdsp_filters/chowdsp_filters.h>

namespace Constants
{
constexpr float fs = 48000.0f;
constexpr float fc = 500.0f;
constexpr float Qval = 0.7071f;
constexpr float gainDB = 6.0f;
} // namespace Constants

class SecondOrderFiltersTest : public TimedUnitTest
{
public:
    SecondOrderFiltersTest() : TimedUnitTest ("Second Order Filters Test", "Filters")
    {
    }

    template <typename T, typename FilterType, typename NumericType = chowdsp::SampleTypeHelpers::NumericType<T>>
    void testFrequency (FilterType& filter, NumericType freq, NumericType expGainDB, NumericType maxError, const juce::String& message)
    {
        auto buffer = test_utils::makeSineWave<NumericType> (freq, Constants::fs, (NumericType) 2);

        juce::HeapBlock<char> dataBlock;
        auto block = test_utils::bufferToBlock<T> (dataBlock, buffer);

        filter.reset();
        filter.processBlock (block.getChannelPointer (0), buffer.getNumSamples());

        test_utils::blockToBuffer<T> (buffer, block);
        const auto halfSamples = buffer.getNumSamples() / 2;
        auto magDB = juce::Decibels::gainToDecibels (buffer.getMagnitude (halfSamples, halfSamples));
        expectWithinAbsoluteError (magDB, expGainDB, maxError, message);
    }

    template <typename T, typename NumericType = chowdsp::SampleTypeHelpers::NumericType<T>>
    void secondOrderLPFTest (NumericType maxError)
    {
        using namespace Constants;

        chowdsp::SecondOrderLPF<T> filter;
        filter.calcCoefs ((T) fc, (T) Qval, (NumericType) fs);

        testFrequency<T> (filter, (NumericType) 1, (NumericType) 0, maxError, "Incorrect gain at low frequencies.");
        testFrequency<T> (filter, (NumericType) fc, (NumericType) -3.01, maxError, "Incorrect gain at cutoff frequency.");
        testFrequency<T> (filter, (NumericType) fc * (NumericType) 4, (NumericType) -24.2, (NumericType) 0.1, "Incorrect gain at high frequencies.");
    }

    template <typename T, typename NumericType = chowdsp::SampleTypeHelpers::NumericType<T>>
    void secondOrderHPFTest (NumericType maxError)
    {
        using namespace Constants;

        chowdsp::SecondOrderHPF<T> filter;
        filter.calcCoefs ((T) fc, (T) Qval, (NumericType) fs);

        testFrequency<T> (filter, (NumericType) fc / (NumericType) 4, (NumericType) -24.1, (NumericType) 0.1, "Incorrect gain at low frequencies.");
        testFrequency<T> (filter, (NumericType) fc, (NumericType) -3.01, maxError, "Incorrect gain at cutoff frequency.");
        testFrequency<T> (filter, (NumericType) fs * (NumericType) 0.498, (NumericType) 0, maxError, "Incorrect gain at high frequencies.");
    }

    template <typename T, typename NumericType = chowdsp::SampleTypeHelpers::NumericType<T>>
    void secondOrderBPFTest (NumericType maxError)
    {
        using namespace Constants;

        chowdsp::SecondOrderBPF<T> filter;
        filter.calcCoefs ((T) fc, (T) Qval, (NumericType) fs);

        testFrequency<T> (filter, (NumericType) fc / (NumericType) 4, (NumericType) -9.0, (NumericType) 0.1, "Incorrect gain at low frequencies.");
        testFrequency<T> (filter, (NumericType) fc, (NumericType) 0, maxError, "Incorrect gain at center frequency.");
        testFrequency<T> (filter, (NumericType) fc * (NumericType) 4, (NumericType) -9.1, (NumericType) 0.1, "Incorrect gain at high frequencies.");
    }

    template <typename T, typename NumericType = chowdsp::SampleTypeHelpers::NumericType<T>>
    void notchFilterTest (NumericType maxError)
    {
        using namespace Constants;

        chowdsp::NotchFilter<T> filter;
        filter.calcCoefs ((T) fc, (T) Qval, (NumericType) fs);

        testFrequency<T> (filter, (NumericType) 1, (NumericType) 0, maxError, "Incorrect gain at low frequencies.");

        if constexpr (std::is_same_v<NumericType, float>)
            testFrequency<T> (filter, (NumericType) fc, (NumericType) -65, (NumericType) 5, "Incorrect gain at center frequency.");
        else
            testFrequency<T> (filter, (NumericType) fc, (NumericType) -100, (NumericType) 5, "Incorrect gain at center frequency.");

        testFrequency<T> (filter, (NumericType) fs * (NumericType) 0.498, (NumericType) 0, maxError, "Incorrect gain at high frequencies.");
    }

    template <typename T, typename NumericType = chowdsp::SampleTypeHelpers::NumericType<T>>
    void peakingFilterTest (NumericType maxError)
    {
        using namespace Constants;

        chowdsp::PeakingFilter<T> filter;
        filter.calcCoefsDB ((T) fc, (T) Qval, (T) gainDB, (NumericType) fs);

        testFrequency<T> (filter, (NumericType) 1, (NumericType) 0, maxError, "Incorrect gain at low frequencies.");
        testFrequency<T> (filter, (NumericType) fc, (NumericType) gainDB, maxError, "Incorrect gain at center frequency.");
        testFrequency<T> (filter, (NumericType) fs * (NumericType) 0.498, (NumericType) 0, maxError, "Incorrect gain at high frequencies.");
    }

    template <typename T, typename NumericType = chowdsp::SampleTypeHelpers::NumericType<T>>
    void lowShelfFilterTest (NumericType maxError)
    {
        using namespace Constants;

        chowdsp::LowShelfFilter<T> filter;
        filter.calcCoefsDB ((T) fc, (T) Qval, (T) gainDB, (NumericType) fs);

        testFrequency<T> (filter, (NumericType) 1, (NumericType) gainDB, maxError, "Incorrect gain at low frequencies.");
        testFrequency<T> (filter, (NumericType) fc, (NumericType) gainDB * (NumericType) 0.5, maxError, "Incorrect gain at cutoff frequency.");
        testFrequency<T> (filter, (NumericType) fs * (NumericType) 0.498, (NumericType) 0, maxError, "Incorrect gain at high frequencies.");
    }

    template <typename T, typename NumericType = chowdsp::SampleTypeHelpers::NumericType<T>>
    void highShelfFilterTest (NumericType maxError)
    {
        using namespace Constants;

        chowdsp::HighShelfFilter<T> filter;
        filter.calcCoefsDB ((T) fc, (T) Qval, (T) gainDB, (NumericType) fs);

        testFrequency<T> (filter, (NumericType) 1, (NumericType) 0, maxError, "Incorrect gain at low frequencies.");
        testFrequency<T> (filter, (NumericType) fc, (NumericType) gainDB * (NumericType) 0.5, maxError, "Incorrect gain at cutoff frequency.");
        testFrequency<T> (filter, (NumericType) fs * (NumericType) 0.498, (NumericType) gainDB, maxError, "Incorrect gain at high frequencies.");
    }

    void runTestTimed() override
    {
        beginTest ("Second Order LPF Test");
        secondOrderLPFTest<float> (1.0e-2f);
        secondOrderLPFTest<double> (1.0e-2);

        beginTest ("Second Order LPF SIMD Test");
        secondOrderLPFTest<xsimd::batch<float>> (1.0e-2f);
        secondOrderLPFTest<xsimd::batch<double>> (1.0e-2);

        beginTest ("Second Order HPF Test");
        secondOrderHPFTest<float> (1.0e-2f);
        secondOrderHPFTest<double> (1.0e-2);

        beginTest ("Second Order HPF SIMD Test");
        secondOrderHPFTest<xsimd::batch<float>> (1.0e-2f);
        secondOrderHPFTest<xsimd::batch<double>> (1.0e-2);

        beginTest ("Second Order BPF Test");
        secondOrderBPFTest<float> (1.0e-2f);
        secondOrderBPFTest<double> (1.0e-2);

        beginTest ("Second Order BPF SIMD Test");
        secondOrderBPFTest<xsimd::batch<float>> (1.0e-2f);
        secondOrderBPFTest<xsimd::batch<double>> (1.0e-2);

        beginTest ("Notch Filter Test");
        notchFilterTest<float> (1.0e-2f);
        notchFilterTest<double> (1.0e-3);

        beginTest ("Notch Filter SIMD Test");
        notchFilterTest<xsimd::batch<float>> (1.0e-2f);
        notchFilterTest<xsimd::batch<double>> (1.0e-3);

        beginTest ("Peaking Filter Test");
        peakingFilterTest<float> (1.0e-2f);
        peakingFilterTest<double> (1.0e-3);

        beginTest ("Peaking Filter SIMD Test");
        peakingFilterTest<xsimd::batch<float>> (1.0e-2f);
        peakingFilterTest<xsimd::batch<double>> (1.0e-3);

        beginTest ("Low Shelf Filter Test");
        lowShelfFilterTest<float> (1.0e-2f);
        lowShelfFilterTest<double> (1.0e-2);

        beginTest ("Low Shelf Filter SIMD Test");
        lowShelfFilterTest<xsimd::batch<float>> (1.0e-2f);
        lowShelfFilterTest<xsimd::batch<double>> (1.0e-2);

        beginTest ("High Shelf Filter Test");
        highShelfFilterTest<float> (1.0e-2f);
        highShelfFilterTest<double> (1.0e-2);

        beginTest ("High Shelf Filter SIMD Test");
        highShelfFilterTest<xsimd::batch<float>> (1.0e-2f);
        highShelfFilterTest<xsimd::batch<double>> (1.0e-2);
    }
};

static SecondOrderFiltersTest secondOrderFiltersTest;
