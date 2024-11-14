#include <chowdsp_dsp_data_structures/chowdsp_dsp_data_structures.h>

using namespace chowdsp::BufferMath;
JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wfloat-equal")

#include <CatchUtils.h>
#include <catch2/generators/catch_generators_all.hpp>

template <typename T, typename BufferType>
static void fillBufferWithOnes (BufferType& buffer)
{
    const auto numChannels = buffer.getNumChannels();
    const auto numSamples = buffer.getNumSamples();

    for (int ch = 0; ch < numChannels; ++ch)
    {
        if constexpr (std::is_floating_point_v<T>)
        {
            juce::FloatVectorOperations::fill (buffer.getWritePointer (ch),
                                               (T) 1,
                                               numSamples);
        }
        else
        {
            std::transform (buffer.getReadPointer (ch),
                            buffer.getReadPointer (ch) + numSamples,
                            buffer.getWritePointer (ch),
                            [] (const auto&)
                            { return (T) 1; });
        }
    }
}

template <typename T1, typename T2>
void copyBufferDataTypesTest()
{
    chowdsp::Buffer<T1> buffer { 2, 4 };
    chowdsp::Buffer<T2> copyBuffer { 2, 4 };

    {
        int count = 0;
        for (auto [channel, data] : chowdsp::buffer_iters::channels (buffer))
        {
            for (auto& x_n : data)
                x_n = (T1) static_cast<float> (count++);
        }
    }

    chowdsp::BufferMath::copyBufferData (buffer, copyBuffer);

    int count = 0;
    for (auto [channel, data] : chowdsp::buffer_iters::channels (std::as_const (copyBuffer)))
    {
        for (auto& x_n : data)
            REQUIRE (chowdsp::SIMDUtils::all ((T2) static_cast<float> (count++) == x_n));
    }
}

TEMPLATE_TEST_CASE ("Buffer Math Test",
                    "[dsp][buffers][simd]",
                    (chowdsp::Buffer<float>),
                    (chowdsp::Buffer<double>),
                    (chowdsp::Buffer<xsimd::batch<float>>),
                    (chowdsp::Buffer<xsimd::batch<double>>),
                    (chowdsp::StaticBuffer<float, 2, 1024>),
                    (chowdsp::StaticBuffer<double, 2, 1024>),
                    (chowdsp::StaticBuffer<xsimd::batch<float>, 2, 1024>),
                    (chowdsp::StaticBuffer<xsimd::batch<double>, 2, 1024>),
                    (juce::AudioBuffer<float>),
                    (juce::AudioBuffer<double>) )
{
    using BufferType = TestType;
    using T = chowdsp::BufferSampleType<BufferType>;
    using NumericType = chowdsp::SampleTypeHelpers::NumericType<T>;
    static constexpr auto maxErr = 1.0e-6;

    SECTION ("Magnitude Test")
    {
        BufferType buffer { 1, 128 };
        buffer.clear();
        REQUIRE_MESSAGE (getMagnitude (buffer) == SIMDApprox<T> ((T) 0).margin ((NumericType) maxErr), "Zero magnitude is incorrect!");

        buffer.getWritePointer (0)[10] = 1.0f;
        REQUIRE_MESSAGE (getMagnitude (buffer) == SIMDApprox<T> ((T) 1).margin ((NumericType) maxErr), "Positive magnitude is incorrect!");

        buffer.clear();
        buffer.getWritePointer (0)[100] = -1.0f;
        REQUIRE_MESSAGE (getMagnitude (buffer) == SIMDApprox<T> ((T) 1).margin ((NumericType) maxErr), "Negative magnitude is incorrect!");
    }

    SECTION ("RMS Test")
    {
        auto&& sineBuffer = test_utils::makeSineWave<T> ((NumericType) 100, (NumericType) 1024, (NumericType) 1);
        REQUIRE_MESSAGE (getRMSLevel (sineBuffer, 0) == SIMDApprox<T> ((T) (NumericType) 0.7071).margin ((NumericType) maxErr),
                         "RMS of a sine wave is incorrect!");
    }

    SECTION ("Copy Buffer Test")
    {
        auto&& srcBuffer = test_utils::makeSineWave<T> ((NumericType) 100, (NumericType) 1024, (NumericType) 1);

        BufferType destBuffer { srcBuffer.getNumChannels(), srcBuffer.getNumSamples() };
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
        const auto rand = Catch::Generators::random ((NumericType) -10, (NumericType) 10);

        BufferType buffer { 2, 128 };
        for (int i = 0; i < buffer.getNumSamples(); ++i)
            buffer.getWritePointer (0)[i] = (T) rand.get();

        copyBufferChannels (buffer, buffer, 0, 1);

        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            const auto actualVal = buffer.getReadPointer (1)[i];
            const auto expVal = buffer.getReadPointer (0)[i];
            REQUIRE_MESSAGE (actualVal == SIMDApprox<T> (expVal).margin ((NumericType) maxErr), "Copied value is incorrect!");
        }
    }

    SECTION ("Add Buffer Test")
    {
        auto&& srcBuffer = test_utils::makeSineWave<T> ((NumericType) 100, (NumericType) 1024, (NumericType) 1);

        BufferType destBuffer { srcBuffer.getNumChannels(), srcBuffer.getNumSamples() };
        for (int i = 0; i < destBuffer.getNumSamples(); ++i)
            destBuffer.getWritePointer (0)[i] = (NumericType) 1;
        addBufferData (srcBuffer, destBuffer);

        for (int i = 0; i < destBuffer.getNumSamples(); ++i)
        {
            const auto actualVal = destBuffer.getReadPointer (0)[i];
            const auto expVal = srcBuffer.getReadPointer (0)[i];
            REQUIRE_MESSAGE (actualVal == SIMDApprox<T> (expVal + (NumericType) 1).margin ((NumericType) maxErr), "Copied value is incorrect!");
        }
    }

    SECTION ("Add Buffer Channel Test")
    {
        const auto rand = Catch::Generators::random ((NumericType) -10, (NumericType) 10);

        BufferType buffer { 2, 128 };
        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            const auto x = (T) rand.get();
            buffer.getWritePointer (0)[i] = x;
            buffer.getWritePointer (1)[i] = x;
        }

        addBufferChannels (buffer, buffer, 0, 1);

        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            const auto actualVal = buffer.getReadPointer (1)[i];
            const auto expVal = buffer.getReadPointer (0)[i];
            REQUIRE_MESSAGE (actualVal == SIMDApprox<T> ((NumericType) 2 * expVal).margin ((NumericType) maxErr), "Copied value is incorrect!");
        }
    }

    SECTION ("Multiply Buffer Test")
    {
        auto&& srcBuffer = test_utils::makeSineWave<T> ((NumericType) 100, (NumericType) 1024, (NumericType) 1);

        BufferType destBuffer { srcBuffer.getNumChannels(), srcBuffer.getNumSamples() };
        for (int i = 0; i < destBuffer.getNumSamples(); ++i)
            destBuffer.getWritePointer (0)[i] = (NumericType) 2;
        multiplyBufferData (srcBuffer, destBuffer);

        for (int i = 0; i < destBuffer.getNumSamples(); ++i)
        {
            const auto actualVal = destBuffer.getReadPointer (0)[i];
            const auto expVal = srcBuffer.getReadPointer (0)[i];
            REQUIRE_MESSAGE (actualVal == SIMDApprox<T> (expVal * (NumericType) 2).margin ((NumericType) maxErr), "Copied value is incorrect!");
        }
    }

    SECTION ("Apply Gain Test")
    {
        auto&& sineBuffer1 = test_utils::makeSineWave<T> ((NumericType) 100, (NumericType) 1024, (NumericType) 1);
        auto&& sineBuffer2 = test_utils::makeSineWave<T> ((NumericType) 100, (NumericType) 1024, (NumericType) 1);

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
            auto&& sineBuffer1 = test_utils::makeSineWave<T> ((NumericType) 100, (NumericType) 1024, (NumericType) 1);
            auto&& sineBuffer2 = test_utils::makeSineWave<T> ((NumericType) 100, (NumericType) 1024, (NumericType) 1);

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
            auto&& sineBuffer1 = test_utils::makeSineWave<T> ((NumericType) 100, (NumericType) 1024, (NumericType) 1);
            auto&& sineBuffer2 = test_utils::makeSineWave<T> ((NumericType) 100, (NumericType) 1024, (NumericType) 1);

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

    if constexpr (std::is_floating_point_v<T>)
    {
        SECTION ("Sanitize Buffer Test")
        {
            BufferType buffer { 2, 100 };

            { // clean buffer
                fillBufferWithOnes<T> (buffer);
                REQUIRE (sanitizeBuffer (buffer));

                const auto* const* data = buffer.getArrayOfReadPointers();
                for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
                {
                    for (int n = 0; n < buffer.getNumSamples(); ++n)
                        REQUIRE (data[ch][n] == (T) 1);
                }
            }

            { // with crazy large values
                fillBufferWithOnes<T> (buffer);
                auto* const* data = buffer.getArrayOfWritePointers();
                data[1][99] = 100000.0f;
                REQUIRE (! sanitizeBuffer (buffer));

                for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
                {
                    for (int n = 0; n < buffer.getNumSamples(); ++n)
                        REQUIRE (data[ch][n] == (T) 0);
                }
            }

            { // with NaN values
                fillBufferWithOnes<T> (buffer);
                auto* const* data = buffer.getArrayOfWritePointers();
                data[0][99] = std::numeric_limits<NumericType>::quiet_NaN();
                REQUIRE (! sanitizeBuffer (buffer));

                for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
                {
                    for (int n = 0; n < buffer.getNumSamples(); ++n)
                        REQUIRE (data[ch][n] == (T) 0);
                }
            }

            { // with INF values
                fillBufferWithOnes<T> (buffer);
                auto* const* data = buffer.getArrayOfWritePointers();
                data[0][2] = std::numeric_limits<NumericType>::infinity();
                REQUIRE (! sanitizeBuffer (buffer));

                for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
                {
                    for (int n = 0; n < buffer.getNumSamples(); ++n)
                        REQUIRE (data[ch][n] == (T) 0);
                }
            }
        }
    }

    SECTION ("Sum To Mono Test")
    {
        BufferType buffer { 2, 100 };
        fillBufferWithOnes<T> (buffer);

        {
            sumToMono (buffer, buffer, (NumericType) 1);
            const auto* const* data = buffer.getArrayOfReadPointers();
            for (int n = 0; n < buffer.getNumSamples(); ++n)
            {
                REQUIRE (chowdsp::SIMDUtils::all (data[0][n] == (T) 2));
                REQUIRE (chowdsp::SIMDUtils::all (data[1][n] == (T) 1));
            }
        }

        {
            BufferType monoBuffer { 1, 100 };
            sumToMono (buffer, monoBuffer);
            const auto* data = monoBuffer.getReadPointer (0);
            for (int n = 0; n < buffer.getNumSamples(); ++n)
            {
                REQUIRE (chowdsp::SIMDUtils::all (data[n] == (T) 1.5));
            }
        }
    }

    SECTION ("Apply Function Test")
    {
        const auto rand = Catch::Generators::random ((NumericType) -10, (NumericType) 10);

        BufferType bufferTest { 1, 128 };
        BufferType bufferRef { 1, 128 };
        for (int i = 0; i < bufferTest.getNumSamples(); ++i)
        {
            const auto x = (T) rand.get();
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
            REQUIRE_MESSAGE (actualVal == SIMDApprox<T> (expVal).margin ((NumericType) maxErr), "Value is incorrect!");
        }
    }

    if constexpr (std::is_floating_point_v<T> && ! std::is_same_v<BufferType, juce::AudioBuffer<T>>)
    {
        SECTION ("Apply Function SIMD Test")
        {
            const auto rand = Catch::Generators::random ((NumericType) -2.5, (NumericType) 2.5);

            BufferType bufferTest { 1, 127 };
            BufferType bufferRef { 1, 127 };
            for (int i = 0; i < bufferTest.getNumSamples(); ++i)
            {
                const auto x = (T) rand.get();
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
                REQUIRE_MESSAGE (actualVal == SIMDApprox<T> (expVal).margin ((NumericType) maxErr), "Value is incorrect!");
            }
        }
    }

    if constexpr (std::is_same_v<BufferType, chowdsp::Buffer<float>>)
    {
        SECTION ("Copy Buffer Data Types")
        {
            copyBufferDataTypesTest<float, double>();
            copyBufferDataTypesTest<double, float>();
        }
    }
}

JUCE_END_IGNORE_WARNINGS_GCC_LIKE
