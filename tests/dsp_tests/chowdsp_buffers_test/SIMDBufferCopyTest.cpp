#include <CatchUtils.h>
#include <chowdsp_dsp_data_structures/chowdsp_dsp_data_structures.h>

TEMPLATE_TEST_CASE ("SIMD Buffer Copy Test", "[dsp][buffers][simd]", float, double)
{
    using T = TestType;

    using chowdsp::SIMDUtils::all;
    auto minus1To1 = test_utils::RandomFloatGenerator { (T) -1, (T) 1 };

    SECTION ("Scalar Buffer to SIMD Buffer")
    {
        chowdsp::Buffer<T> scalarBuffer { 3, 128 };

        for (int ch = 0; ch < scalarBuffer.getNumChannels(); ++ch)
        {
            for (int n = 0; n < scalarBuffer.getNumSamples(); ++n)
                scalarBuffer.getWritePointer (ch)[n] = minus1To1();
        }

        chowdsp::Buffer<xsimd::batch<T>> simdBuffer { 2, 256 };
        chowdsp::copyToSIMDBuffer (scalarBuffer, simdBuffer);

        REQUIRE_MESSAGE (simdBuffer.getNumSamples() == scalarBuffer.getNumSamples(), "SIMD buffer # samples is incorrect!");

        if constexpr (std::is_same_v<T, float>)
            REQUIRE_MESSAGE (simdBuffer.getNumChannels() == 1, "SIMD buffer # channels is incorrect!");
        else if constexpr (std::is_same_v<T, double>)
            REQUIRE_MESSAGE (simdBuffer.getNumChannels() == 2, "SIMD buffer # channels is incorrect!");

        for (int ch = 0; ch < simdBuffer.getNumChannels(); ++ch)
        {
            for (int n = 0; n < simdBuffer.getNumSamples(); ++n)
            {
                for (int i = 0; i < (int) xsimd::batch<T>::size; ++i)
                {
                    const auto scalarChIndex = ch * (int) xsimd::batch<T>::size + i;
                    REQUIRE_MESSAGE (juce::exactlyEqual (simdBuffer.getReadPointer (ch)[n].get ((size_t) i),
                                                         scalarChIndex < scalarBuffer.getNumChannels()
                                                             ? scalarBuffer.getReadPointer (scalarChIndex)[n]
                                                             : 0.0f),
                                     std::string ("Failure at channel ") + std::to_string (ch)
                                         + std::string (", sample ") + std::to_string (n)
                                         + std::string (", index ") + std::to_string (i));
                }
            }
        }
    }

    SECTION ("SIMD Buffer to Scalar Buffer")
    {
        chowdsp::Buffer<xsimd::batch<T>> simdBuffer { 2, 128 };

        for (int ch = 0; ch < simdBuffer.getNumChannels(); ++ch)
        {
            for (int n = 0; n < simdBuffer.getNumSamples(); ++n)
            {
                alignas (chowdsp::SIMDUtils::defaultSIMDAlignment) T vec[xsimd::batch<T>::size];
                for (int i = 0; i < (int) xsimd::batch<T>::size; ++i)
                    vec[i] = minus1To1();
                simdBuffer.getWritePointer (ch)[n] = xsimd::load_aligned (vec);
            }
        }

        chowdsp::Buffer<T> scalarBuffer { 2 * (int) xsimd::batch<T>::size, 128 };

        chowdsp::copyFromSIMDBuffer (simdBuffer, scalarBuffer);

        REQUIRE_MESSAGE (scalarBuffer.getNumSamples() == simdBuffer.getNumSamples(), "Scalar buffer # samples is incorrect!");

        if constexpr (std::is_same_v<T, float>)
            REQUIRE_MESSAGE (scalarBuffer.getNumChannels() == 8, "Scalar buffer # channels is incorrect!");
        else if constexpr (std::is_same_v<T, double>)
            REQUIRE_MESSAGE (scalarBuffer.getNumChannels() == 4, "Scalar buffer # channels is incorrect!");

        for (int ch = 0; ch < scalarBuffer.getNumChannels(); ++ch)
        {
            for (int n = 0; n < scalarBuffer.getNumSamples(); ++n)
            {
                REQUIRE_MESSAGE (juce::exactlyEqual (scalarBuffer.getReadPointer (ch)[n],
                                                     simdBuffer.getReadPointer (ch / (int) xsimd::batch<T>::size)[n]
                                                         .get ((size_t) ch % xsimd::batch<T>::size)),
                                 std::string ("Failure at channel ") + std::to_string (ch)
                                     + std::string (", sample ") + std::to_string (n));
            }
        }
    }

    SECTION ("Scalar Buffer to SIMD Buffer and back to Scalar Buffer")
    {
        chowdsp::Buffer<T> scalarBufferRef { 3, 128 };

        for (int ch = 0; ch < scalarBufferRef.getNumChannels(); ++ch)
        {
            for (int n = 0; n < scalarBufferRef.getNumSamples(); ++n)
                scalarBufferRef.getWritePointer (ch)[n] = minus1To1();
        }

        chowdsp::Buffer<T> scalarBufferTest { 3, 128 };
        chowdsp::BufferMath::copyBufferData (scalarBufferRef, scalarBufferTest);

        chowdsp::Buffer<xsimd::batch<T>> simdBuffer { 2, 256 };
        chowdsp::copyToSIMDBuffer (scalarBufferTest, simdBuffer);
        chowdsp::BufferMath::applyGain (simdBuffer, 2.0f);
        chowdsp::copyFromSIMDBuffer (simdBuffer, scalarBufferTest);

        REQUIRE_MESSAGE (scalarBufferRef.getNumChannels() == scalarBufferTest.getNumChannels(),
                         "Buffer # channels is incorrect!");
        REQUIRE_MESSAGE (scalarBufferRef.getNumSamples() == scalarBufferTest.getNumSamples(),
                         "Buffer # channels is incorrect!");

        for (int ch = 0; ch < scalarBufferTest.getNumChannels(); ++ch)
        {
            for (int n = 0; n < scalarBufferTest.getNumSamples(); ++n)
            {
                REQUIRE_MESSAGE (juce::exactlyEqual (scalarBufferTest.getReadPointer (ch)[n],
                                                     scalarBufferRef.getReadPointer (ch)[n] * 2.0f),
                                 std::string ("Failure at channel ") + std::to_string (ch)
                                     + std::string (", sample ") + std::to_string (n));
            }
        }
    }
}
