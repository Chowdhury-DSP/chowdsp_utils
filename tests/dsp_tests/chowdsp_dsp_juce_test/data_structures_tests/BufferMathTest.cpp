#define CATCH_CONFIG_RUNNER
#include <TimedUnitTest.h>
#include <CatchUtils.h>
#include <chowdsp_dsp_data_structures/chowdsp_dsp_data_structures.h>

using namespace chowdsp::BufferMath;

template <typename BufferType>
class BufferMathTest : public TimedUnitTest
{
    using T = detail::BufferSampleType<BufferType>;
    using NumericType = chowdsp::SampleTypeHelpers::NumericType<T>;
    static constexpr auto maxErr = 1.0e-6;

public:
    explicit BufferMathTest (const juce::String& bufferTypeName)
        : TimedUnitTest ("Buffer Math Test [" + bufferTypeName + "]", "Buffers")
    {
    }

    void magnitudeTest()
    {
        BufferType buffer { 1, 128 };
        buffer.clear();
        expect (getMagnitude (buffer) == SIMDApprox<T> ((T) 0).margin ((NumericType) maxErr), "Zero magnitude is incorrect!");

        buffer.getWritePointer (0)[10] = 1.0f;
        expect (getMagnitude (buffer) == SIMDApprox<T> ((T) 1).margin ((NumericType) maxErr), "Positive magnitude is incorrect!");

        buffer.clear();
        buffer.getWritePointer (0)[100] = -1.0f;
        expect (getMagnitude (buffer) == SIMDApprox<T> ((T) 1).margin ((NumericType) maxErr), "Negative magnitude is incorrect!");
    }

    void rmsTest()
    {
        auto&& sineBuffer = test_utils::makeSineWave<T> ((NumericType) 100, (NumericType) 4800, (NumericType) 1);
        expect (getRMSLevel (sineBuffer, 0) == SIMDApprox<T> ((T) (NumericType) 0.7071).margin ((NumericType) maxErr), "RMS of a sine wave is incorrect!");
    }

    void copyBufferTest()
    {
        auto&& srcBuffer = test_utils::makeSineWave<T> ((NumericType) 100, (NumericType) 4800, (NumericType) 1);

        BufferType destBuffer { srcBuffer.getNumChannels(), srcBuffer.getNumSamples() };
        copyBufferData (srcBuffer, destBuffer);

        for (int i = 0; i < destBuffer.getNumSamples(); ++i)
        {
            const auto actualVal = destBuffer.getReadPointer (0)[i];
            const auto expVal = srcBuffer.getReadPointer (0)[i];
            expect (actualVal == SIMDApprox<T> (expVal).margin ((NumericType) maxErr), "Copied value is incorrect!");
        }
    }

    void copyBufferChannelTest()
    {
        std::random_device rd;
        std::mt19937 mt (rd());
        std::uniform_real_distribution<NumericType> minu1To1 ((NumericType) -10, (NumericType) 10);

        BufferType buffer { 2, 128 };
        for (int i = 0; i < buffer.getNumSamples(); ++i)
            buffer.getWritePointer (0)[i] = (T) minu1To1 (mt);

        copyBufferChannels (buffer, buffer, 0, 1);

        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            const auto actualVal = buffer.getReadPointer (1)[i];
            const auto expVal = buffer.getReadPointer (0)[i];
            expect (actualVal == SIMDApprox<T> (expVal).margin ((NumericType) maxErr), "Copied value is incorrect!");
        }
    }

    void addBufferTest()
    {
        auto&& srcBuffer = test_utils::makeSineWave<T> ((NumericType) 100, (NumericType) 4800, (NumericType) 1);

        BufferType destBuffer { srcBuffer.getNumChannels(), srcBuffer.getNumSamples() };
        for (int i = 0; i < destBuffer.getNumSamples(); ++i)
            destBuffer.getWritePointer (0)[i] = (NumericType) 1;
        addBufferData (srcBuffer, destBuffer);

        for (int i = 0; i < destBuffer.getNumSamples(); ++i)
        {
            const auto actualVal = destBuffer.getReadPointer (0)[i];
            const auto expVal = srcBuffer.getReadPointer (0)[i];
            expect (actualVal == SIMDApprox<T> (expVal + (NumericType) 1).margin ((NumericType) maxErr), "Copied value is incorrect!");
        }
    }

    void addBufferChannelTest()
    {
        std::random_device rd;
        std::mt19937 mt (rd());
        std::uniform_real_distribution<NumericType> minu1To1 ((NumericType) -10, (NumericType) 10);

        BufferType buffer { 2, 128 };
        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            const auto x = (T) minu1To1 (mt);
            buffer.getWritePointer (0)[i] = x;
            buffer.getWritePointer (1)[i] = x;
        }

        addBufferChannels (buffer, buffer, 0, 1);

        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            const auto actualVal = buffer.getReadPointer (1)[i];
            const auto expVal = buffer.getReadPointer (0)[i];
            expect (actualVal == SIMDApprox<T> ((NumericType) 2 * expVal).margin ((NumericType) maxErr), "Copied value is incorrect!");
        }
    }

    void applyGainTest()
    {
        auto&& sineBuffer1 = test_utils::makeSineWave<T> ((NumericType) 100, (NumericType) 4800, (NumericType) 1);
        auto&& sineBuffer2 = test_utils::makeSineWave<T> ((NumericType) 100, (NumericType) 4800, (NumericType) 1);

        applyGain (sineBuffer1, (NumericType) 2);

        for (int i = 0; i < sineBuffer1.getNumSamples(); ++i)
        {
            const auto actualVal = sineBuffer1.getReadPointer (0)[i];
            const auto expVal = sineBuffer2.getReadPointer (0)[i] * (NumericType) 2;
            expect (actualVal == SIMDApprox<T> (expVal).margin ((NumericType) maxErr), "Value with gain applied is incorrect!");
        }
    }

    void applyGainSmoothedTest()
    {
        for (auto target : { (NumericType) 0, (NumericType) 1 })
        {
            auto&& sineBuffer1 = test_utils::makeSineWave<T> ((NumericType) 100, (NumericType) 4800, (NumericType) 1);
            auto&& sineBuffer2 = test_utils::makeSineWave<T> ((NumericType) 100, (NumericType) 4800, (NumericType) 1);

            juce::SmoothedValue<NumericType> sm1;
            sm1.setCurrentAndTargetValue ((NumericType) 0);
            sm1.reset (10000);
            sm1.setTargetValue (target);

            juce::SmoothedValue<NumericType> sm2;
            sm2.setCurrentAndTargetValue ((NumericType) 0);
            sm2.reset (10000);
            sm2.setTargetValue (target);

            applyGainSmoothed (sineBuffer1, sm1);

            for (int i = 0; i < sineBuffer1.getNumSamples(); ++i)
            {
                const auto actualVal = sineBuffer1.getReadPointer (0)[i];
                const auto expVal = sineBuffer2.getReadPointer (0)[i] * sm2.getNextValue();
                expect (actualVal == SIMDApprox<T> (expVal).margin ((NumericType) maxErr), "Value with smoothed gain applied is incorrect!");
            }
        }
    }

    void applyGainSmoothedBufferTest()
    {
        for (auto target : { (NumericType) 0, (NumericType) -1 })
        {
            auto&& sineBuffer1 = test_utils::makeSineWave<T> ((NumericType) 100, (NumericType) 4800, (NumericType) 1);
            auto&& sineBuffer2 = test_utils::makeSineWave<T> ((NumericType) 100, (NumericType) 4800, (NumericType) 1);

            chowdsp::SmoothedBufferValue<NumericType> sm1;
            sm1.prepare ((NumericType) 48000, sineBuffer1.getNumSamples());
            sm1.reset ((NumericType) 0);
            sm1.process (target, sineBuffer1.getNumSamples());

            chowdsp::SmoothedBufferValue<NumericType> sm2;
            sm2.prepare ((NumericType) 48000, sineBuffer2.getNumSamples());
            sm2.reset ((NumericType) 0);
            sm2.process (target, sineBuffer2.getNumSamples());

            applyGainSmoothedBuffer (sineBuffer1, sm1);

            for (int i = 0; i < sineBuffer1.getNumSamples(); ++i)
            {
                const auto actualVal = sineBuffer1.getReadPointer (0)[i];
                const auto expVal = sineBuffer2.getReadPointer (0)[i] * sm2.getSmoothedBuffer()[i];
                expect (actualVal == SIMDApprox<T> (expVal).margin ((NumericType) maxErr), "Value with smoothed buffer gain applied is incorrect!");
            }
        }
    }

    static void fillBufferWithOnes (BufferType& buffer)
    {
        const auto numChannels = buffer.getNumChannels();
        const auto numSamples = buffer.getNumSamples();

        for (int ch = 0; ch < numChannels; ++ch)
            juce::FloatVectorOperations::fill (buffer.getWritePointer (ch), (T) 1, numSamples);
    }

    void sanitizeBufferTest()
    {
        BufferType buffer { 2, 100 };

        { // clean buffer
            fillBufferWithOnes (buffer);
            expect (sanitizeBuffer (buffer));

            const auto** data = buffer.getArrayOfReadPointers();
            for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
            {
                for (int n = 0; n < buffer.getNumSamples(); ++n)
                    expect (data[ch][n] == (T) 1);
            }
        }

        { // with crazy large values
            fillBufferWithOnes (buffer);
            auto** data = buffer.getArrayOfWritePointers();
            data[1][99] = 100000.0f;
            expect (! sanitizeBuffer (buffer));

            for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
            {
                for (int n = 0; n < buffer.getNumSamples(); ++n)
                    expect (data[ch][n] == (T) 0);
            }
        }

        { // with NaN values
            fillBufferWithOnes (buffer);
            auto** data = buffer.getArrayOfWritePointers();
            data[0][99] = std::numeric_limits<NumericType>::quiet_NaN();
            expect (! sanitizeBuffer (buffer));

            for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
            {
                for (int n = 0; n < buffer.getNumSamples(); ++n)
                    expect (data[ch][n] == (T) 0);
            }
        }

        { // with INF values
            fillBufferWithOnes (buffer);
            auto** data = buffer.getArrayOfWritePointers();
            data[0][2] = std::numeric_limits<NumericType>::infinity();
            expect (! sanitizeBuffer (buffer));

            for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
            {
                for (int n = 0; n < buffer.getNumSamples(); ++n)
                    expect (data[ch][n] == (T) 0);
            }
        }
    }

    void applyFunctionTest()
    {
        std::random_device rd;
        std::mt19937 mt (rd());
        std::uniform_real_distribution<NumericType> minus10To10 ((NumericType) -10, (NumericType) 10);

        BufferType bufferTest { 1, 128 };
        BufferType bufferRef { 1, 128 };
        for (int i = 0; i < bufferTest.getNumSamples(); ++i)
        {
            const auto x = (T) minus10To10 (mt);
            bufferTest.getWritePointer (0)[i] = x;
            bufferRef.getWritePointer (0)[i] = x;
        }

        applyFunction (bufferTest,
                       [] (auto x)
                       {
                           CHOWDSP_USING_XSIMD_STD (tanh);
                           return tanh (x);
                       });

        for (int i = 0; i < bufferTest.getNumSamples(); ++i)
        {
            CHOWDSP_USING_XSIMD_STD (tanh);
            const auto actualVal = bufferTest.getReadPointer (0)[i];
            const auto expVal = tanh (bufferRef.getReadPointer (0)[i]);
            expect (actualVal == SIMDApprox<T> (expVal).margin ((NumericType) maxErr), "Value is incorrect!");
        }
    }

    void applyFunctionSIMDTest()
    {
        std::random_device rd;
        std::mt19937 mt (rd());
        std::uniform_real_distribution<NumericType> minus10To10 ((NumericType) -2.5, (NumericType) 2.5);

        BufferType bufferTest { 1, 127 };
        BufferType bufferRef { 1, 127 };
        for (int i = 0; i < bufferTest.getNumSamples(); ++i)
        {
            const auto x = (T) minus10To10 (mt);
            bufferTest.getWritePointer (0)[i] = x;
            bufferRef.getWritePointer (0)[i] = x;
        }

        applyFunctionSIMD (bufferTest,
                           [] (auto x)
                           {
                               CHOWDSP_USING_XSIMD_STD (tanh);
                               return tanh (x);
                           });

        for (int i = 0; i < bufferTest.getNumSamples(); ++i)
        {
            CHOWDSP_USING_XSIMD_STD (tanh);
            const auto actualVal = bufferTest.getReadPointer (0)[i];
            const auto expVal = tanh (bufferRef.getReadPointer (0)[i]);
            expect (actualVal == SIMDApprox<T> (expVal).margin ((NumericType) maxErr), "Value is incorrect!");
        }
    }

    void runTestTimed() override
    {
        beginTest ("Magnitude Test");
        magnitudeTest();

        beginTest ("RMS Test");
        rmsTest();

        beginTest ("Copy Buffer Test");
        copyBufferTest();

        beginTest ("Copy Buffer Channel Test");
        copyBufferChannelTest();

        beginTest ("Add Buffer Test");
        addBufferTest();

        beginTest ("Add Buffer Channel Test");
        addBufferChannelTest();

        beginTest ("Apply Gain Test");
        applyGainTest();

        beginTest ("Apply Gain Smoothed Test");
        applyGainSmoothedTest();

        beginTest ("Apply Gain Smoothed Buffer Test");
        applyGainSmoothedBufferTest();

        if constexpr (std::is_floating_point_v<T>)
        {
            beginTest ("Sanitize Buffer Test");
            sanitizeBufferTest();
        }

        beginTest ("Apply Function Test");
        applyFunctionTest();

        if constexpr (std::is_floating_point_v<T> && ! std::is_same_v<BufferType, juce::AudioBuffer<T>>)
        {
            beginTest ("Apply Function SIMD Test");
            applyFunctionSIMDTest();
        }
    }
};

static BufferMathTest<chowdsp::Buffer<float>> bufferFloatMathTest { "chowdsp::Buffer<float>" };
static BufferMathTest<chowdsp::Buffer<double>> bufferDoubleMathTest { "chowdsp::Buffer<double>" };
static BufferMathTest<chowdsp::Buffer<xsimd::batch<float>>> bufferFloatVecMathTest { "chowdsp::Buffer<xsimd::batch<float>>" };
static BufferMathTest<chowdsp::Buffer<xsimd::batch<double>>> bufferDoubleVecMathTest { "chowdsp::Buffer<xsimd::batch<double>>" };
static BufferMathTest<chowdsp::StaticBuffer<float, 2, 4800>> staticBufferFloatMathTest { "chowdsp::StaticBuffer<float>" };
static BufferMathTest<chowdsp::StaticBuffer<double, 2, 4800>> staticBufferDoubleMathTest { "chowdsp::StaticBuffer<double>" };
static BufferMathTest<chowdsp::StaticBuffer<xsimd::batch<float>, 2, 4800>> staticBufferFloatVecMathTest { "chowdsp::StaticBuffer<xsimd::batch<float>>" };
static BufferMathTest<chowdsp::StaticBuffer<xsimd::batch<double>, 2, 4800>> staticBufferDoubleVecMathTest { "chowdsp::StaticBuffer<xsimd::batch<double>>" };
static BufferMathTest<juce::AudioBuffer<float>> juceBufferFloatMathTest { "juce::AudioBuffer<float>" };
static BufferMathTest<juce::AudioBuffer<double>> juceBufferDoubleMathTest { "juce::AudioBuffer<double>" };
