#include <test_utils.h>
#include <TimedUnitTest.h>

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

    template <typename T, typename FilterType, typename NumericType = typename chowdsp::SampleTypeHelpers::ElementType<T>::Type>
    void testFrequency (FilterType& filter, NumericType freq, NumericType expGainDB, NumericType maxError, const String& message)
    {
        auto buffer = test_utils::makeSineWave<NumericType> (freq, Constants::fs, (NumericType) 2);

        HeapBlock<char> dataBlock;
        auto block = test_utils::bufferToBlock<T> (dataBlock, buffer);

        filter.reset();
        filter.processBlock (block.getChannelPointer (0), buffer.getNumSamples());

        test_utils::blockToBuffer (buffer, block);
        const auto halfSamples = buffer.getNumSamples() / 2;
        auto magDB = Decibels::gainToDecibels (buffer.getMagnitude (halfSamples, halfSamples));
        expectWithinAbsoluteError (magDB, expGainDB, maxError, message);
    }

    template <typename T, typename NumericType = typename chowdsp::SampleTypeHelpers::ElementType<T>::Type>
    void secondOrderLPFTest (NumericType maxError)
    {
        using namespace Constants;

        chowdsp::SecondOrderLPF<T> filter;
        filter.calcCoefs ((T) fc, (T) Qval, (NumericType) fs);

        testFrequency<T> (filter, (NumericType) 1, (NumericType) 0, maxError, "Incorrect gain at low frequencies.");
        testFrequency<T> (filter, (NumericType) fc, (NumericType) -3.01, maxError, "Incorrect gain at cutoff frequency.");
        testFrequency<T> (filter, (NumericType) fc * (NumericType) 4, (NumericType) -24.2, (NumericType) 0.1, "Incorrect gain at high frequencies.");
    }

    template <typename T, typename NumericType = typename chowdsp::SampleTypeHelpers::ElementType<T>::Type>
    void secondOrderHPFTest (NumericType maxError)
    {
        using namespace Constants;

        chowdsp::SecondOrderHPF<T> filter;
        filter.calcCoefs ((T) fc, (T) Qval, (NumericType) fs);

        testFrequency<T> (filter, (NumericType) fc / (NumericType) 4, (NumericType) -24.0, (NumericType) 0.1, "Incorrect gain at low frequencies.");
        testFrequency<T> (filter, (NumericType) fc, (NumericType) -3.0, maxError, "Incorrect gain at cutoff frequency.");
        testFrequency<T> (filter, (NumericType) fs * (NumericType) 0.498, (NumericType) 0, maxError, "Incorrect gain at high frequencies.");
    }

    template <typename T, typename NumericType = typename chowdsp::SampleTypeHelpers::ElementType<T>::Type>
    void secondOrderBPFTest (NumericType maxError)
    {
        using namespace Constants;

        chowdsp::SecondOrderBPF<T> filter;
        filter.calcCoefs ((T) fc, (T) Qval, (NumericType) fs);

        testFrequency<T> (filter, (NumericType) fc / (NumericType) 4, (NumericType) -9.0, (NumericType) 0.1, "Incorrect gain at low frequencies.");
        testFrequency<T> (filter, (NumericType) fc, (NumericType) 0, maxError, "Incorrect gain at center frequency.");
        testFrequency<T> (filter, (NumericType) fc * (NumericType) 4, (NumericType) -9.1, (NumericType) 0.1, "Incorrect gain at high frequencies.");
    }

    template <typename T, typename NumericType = typename chowdsp::SampleTypeHelpers::ElementType<T>::Type>
    void notchFilterTest (NumericType maxError)
    {
        using namespace Constants;

        chowdsp::NotchFilter<T> filter;
        filter.calcCoefs ((T) fc, (T) Qval, (NumericType) fs);

        testFrequency<T> (filter, (NumericType) 1, (NumericType) 0, maxError, "Incorrect gain at low frequencies.");
        testFrequency<T> (filter, (NumericType) fc, (NumericType) -60, (NumericType) 5, "Incorrect gain at center frequency.");
        testFrequency<T> (filter, (NumericType) fs * (NumericType) 0.498, (NumericType) 0, maxError, "Incorrect gain at high frequencies.");
    }

    template <typename T, typename NumericType = typename chowdsp::SampleTypeHelpers::ElementType<T>::Type>
    void peakingFilterTest (NumericType maxError)
    {
        using namespace Constants;

        chowdsp::PeakingFilter<T> filter;
        filter.calcCoefsDB ((T) fc, (T) Qval, (T) gainDB, (NumericType) fs);

        testFrequency<T> (filter, (NumericType) 1, (NumericType) 0, maxError, "Incorrect gain at low frequencies.");
        testFrequency<T> (filter, (NumericType) fc, (NumericType) gainDB, maxError, "Incorrect gain at center frequency.");
        testFrequency<T> (filter, (NumericType) fs * (NumericType) 0.498, (NumericType) 0, maxError, "Incorrect gain at high frequencies.");
    }

    template <typename T, typename NumericType = typename chowdsp::SampleTypeHelpers::ElementType<T>::Type>
    void lowShelfFilterTest (NumericType maxError)
    {
        using namespace Constants;

        chowdsp::LowShelfFilter<T> filter;
        filter.calcCoefsDB ((T) fc, (T) Qval, (T) gainDB, (NumericType) fs);

        testFrequency<T> (filter, (NumericType) 1, (NumericType) gainDB, maxError, "Incorrect gain at low frequencies.");
        testFrequency<T> (filter, (NumericType) fc, (NumericType) gainDB * (NumericType) 0.5, maxError, "Incorrect gain at cutoff frequency.");
        testFrequency<T> (filter, (NumericType) fs * (NumericType) 0.498, (NumericType) 0, maxError, "Incorrect gain at high frequencies.");
    }

    template <typename T, typename NumericType = typename chowdsp::SampleTypeHelpers::ElementType<T>::Type>
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
        secondOrderLPFTest<dsp::SIMDRegister<float>> (1.0e-2f);
        secondOrderLPFTest<dsp::SIMDRegister<double>> (1.0e-2);

        beginTest ("Second Order HPF Test");
        secondOrderHPFTest<float> (1.0e-2f);
        secondOrderHPFTest<double> (1.0e-2);

        beginTest ("Second Order HPF SIMD Test");
        secondOrderHPFTest<dsp::SIMDRegister<float>> (1.0e-2f);
        secondOrderHPFTest<dsp::SIMDRegister<double>> (1.0e-2);

        beginTest ("Second Order BPF Test");
        secondOrderBPFTest<float> (1.0e-2f);
        secondOrderBPFTest<double> (1.0e-2);

        beginTest ("Second Order BPF SIMD Test");
        secondOrderBPFTest<dsp::SIMDRegister<float>> (1.0e-2f);
        secondOrderBPFTest<dsp::SIMDRegister<double>> (1.0e-2);

        beginTest ("Notch Filter Test");
        notchFilterTest<float> (1.0e-2f);
        notchFilterTest<double> (1.0e-3);

        beginTest ("Notch Filter SIMD Test");
        notchFilterTest<dsp::SIMDRegister<float>> (1.0e-2f);
        notchFilterTest<dsp::SIMDRegister<double>> (1.0e-3);

        beginTest ("Peaking Filter Test");
        peakingFilterTest<float> (1.0e-2f);
        peakingFilterTest<double> (1.0e-3);

        beginTest ("Peaking Filter SIMD Test");
        peakingFilterTest<dsp::SIMDRegister<float>> (1.0e-2f);
        peakingFilterTest<dsp::SIMDRegister<double>> (1.0e-3);

        beginTest ("Low Shelf Filter Test");
        lowShelfFilterTest<float> (1.0e-2f);
        lowShelfFilterTest<double> (1.0e-2);

        beginTest ("Low Shelf Filter SIMD Test");
        lowShelfFilterTest<dsp::SIMDRegister<float>> (1.0e-2f);
        lowShelfFilterTest<dsp::SIMDRegister<double>> (1.0e-2);

        beginTest ("High Shelf Filter Test");
        highShelfFilterTest<float> (1.0e-2f);
        highShelfFilterTest<double> (1.0e-2);

        beginTest ("High Shelf Filter SIMD Test");
        highShelfFilterTest<dsp::SIMDRegister<float>> (1.0e-2f);
        highShelfFilterTest<dsp::SIMDRegister<double>> (1.0e-2);
    }
};

static SecondOrderFiltersTest secondOrderFiltersTest;
