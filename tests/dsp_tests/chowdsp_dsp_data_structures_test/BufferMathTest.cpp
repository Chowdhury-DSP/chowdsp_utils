#include <CatchUtils.h>
#include <chowdsp_dsp_data_structures/chowdsp_dsp_data_structures.h>

TEMPLATE_TEST_CASE ("Buffer Math Test", "", float, double, xsimd::batch<float>, xsimd::batch<double>)
{
    using T = TestType;
    using NumericType = chowdsp::SampleTypeHelpers::NumericType<T>;
    static constexpr auto maxErr = 1.0e-6;

    using namespace chowdsp::BufferMath;

    SECTION ("Magnitude Test")
    {
        chowdsp::Buffer<T> buffer { 1, 128 };
        REQUIRE_MESSAGE (getMagnitude (buffer) == SIMDApprox<T> ((T) 0).margin ((NumericType) maxErr), "Zero magnitude is incorrect!");

        buffer.getWritePointer (0)[10] = 1.0f;
        REQUIRE_MESSAGE (getMagnitude (buffer) == SIMDApprox<T> ((T) 1).margin ((NumericType) maxErr), "Positive magnitude is incorrect!");

        buffer.clear();
        buffer.getWritePointer (0)[100] = -1.0f;
        REQUIRE_MESSAGE (getMagnitude (buffer) == SIMDApprox<T> ((T) 1).margin ((NumericType) maxErr), "Negative magnitude is incorrect!");
    }

    SECTION ("RMS Test")
    {
        auto&& sineBuffer = test_utils::makeSineWave<T> ((NumericType) 100, (NumericType) 48000, (NumericType) 1);
        REQUIRE_MESSAGE (getRMSLevel (sineBuffer, 0) == SIMDApprox<T> ((T) (NumericType) 0.7071).margin ((NumericType) maxErr), "RMS of a sine wave is incorrect!");
    }

    SECTION ("Copy Buffer Test")
    {
        auto&& srcBuffer = test_utils::makeSineWave<T> ((NumericType) 100, (NumericType) 48000, (NumericType) 1);

        chowdsp::Buffer<T> destBuffer { srcBuffer.getNumChannels(), srcBuffer.getNumSamples() };
        copyBufferData (srcBuffer, destBuffer);

        for (int i = 0; i < destBuffer.getNumSamples(); ++i)
        {
            const auto actualVal = destBuffer.getReadPointer (0)[i];
            const auto expVal = srcBuffer.getReadPointer (0)[i];
            REQUIRE_MESSAGE (actualVal == SIMDApprox<T> (expVal).margin ((NumericType) maxErr), "Copied value is incorrect!");
        }
    }

    SECTION ("Copy Buffer Channel Test")
    {
        std::random_device rd;
        std::mt19937 mt (rd());
        std::uniform_real_distribution<NumericType> minu1To1 ((NumericType) -10, (NumericType) 10);

        chowdsp::Buffer<T> buffer { 2, 128 };
        for (int i = 0; i < buffer.getNumSamples(); ++i)
            buffer.getWritePointer (0)[i] = (T) minu1To1 (mt);

        copyBufferChannels (buffer, buffer, 0, 1);

        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            const auto actualVal = buffer.getReadPointer (1)[i];
            const auto expVal = buffer.getReadPointer (0)[i];
            REQUIRE_MESSAGE (actualVal == SIMDApprox<T> (expVal).margin ((NumericType) maxErr), "Copied value is incorrect!");
        }
    }

    SECTION ("Apply Gain Test")
    {
        auto&& sineBuffer1 = test_utils::makeSineWave<T> ((NumericType) 100, (NumericType) 48000, (NumericType) 1);
        auto&& sineBuffer2 = test_utils::makeSineWave<T> ((NumericType) 100, (NumericType) 48000, (NumericType) 1);

        applyGain (sineBuffer1, (NumericType) 2);

        for (int i = 0; i < sineBuffer1.getNumSamples(); ++i)
        {
            const auto actualVal = sineBuffer1.getReadPointer (0)[i];
            const auto expVal = sineBuffer2.getReadPointer (0)[i] * (NumericType) 2;
            REQUIRE_MESSAGE (actualVal == SIMDApprox<T> (expVal).margin ((NumericType) maxErr), "Value with gain applied is incorrect!");
        }
    }

    SECTION ("Apply Gain Smoothed Test")
    {
        for (auto target : { (NumericType) 0, (NumericType) 1 })
        {
            auto&& sineBuffer1 = test_utils::makeSineWave<T> ((NumericType) 100, (NumericType) 48000, (NumericType) 1);
            auto&& sineBuffer2 = test_utils::makeSineWave<T> ((NumericType) 100, (NumericType) 48000, (NumericType) 1);

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
                REQUIRE_MESSAGE (actualVal == SIMDApprox<T> (expVal).margin ((NumericType) maxErr), "Value with smoothed gain applied is incorrect!");
            }
        }
    }

    SECTION ("Apply Gain Smoothed Buffer Test")
    {
        for (auto target : { (NumericType) 0, (NumericType) -1 })
        {
            auto&& sineBuffer1 = test_utils::makeSineWave<T> ((NumericType) 100, (NumericType) 48000, (NumericType) 1);
            auto&& sineBuffer2 = test_utils::makeSineWave<T> ((NumericType) 100, (NumericType) 48000, (NumericType) 1);

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
                REQUIRE_MESSAGE (actualVal == SIMDApprox<T> (expVal).margin ((NumericType) maxErr), "Value with smoothed buffer gain applied is incorrect!");
            }
        }
    }
}
