#include <CatchUtils.h>
#include <juce_dsp/juce_dsp.h>
#include <chowdsp_filters/chowdsp_filters.h>

TEST_CASE ("FIR Filter Test")
{
    SECTION ("FIR Filter Test")
    {
        static constexpr double fs = 48000.0;
        static constexpr int blockSize = 512;
        static constexpr int firOrder = 63;

        auto&& coeffsBuffer = test_utils::makeNoise<float> (firOrder);
        auto&& actualBuffer = test_utils::makeSineWave<float> (100.0f, (float) fs, blockSize);
        auto&& refBuffer = juce::AudioBuffer<float> { actualBuffer.getArrayOfWritePointers(), 1, blockSize };

        {
            juce::dsp::FIR::Filter<float> refFilter;
            auto& coefs = refFilter.coefficients->coefficients;
            coefs.resize (firOrder);
            auto* coeffsBufferData = coeffsBuffer.getReadPointer (0);
            std::copy (coeffsBufferData, coeffsBufferData + firOrder, coefs.begin());

            refFilter.prepare ({ fs, (juce::uint32) blockSize, 1 });

            auto&& refBlock = juce::dsp::AudioBlock<float> { refBuffer };
            refFilter.process (juce::dsp::ProcessContextReplacing<float> { refBlock });
        }

        {
            chowdsp::FIRFilter<float> filter { firOrder };
            filter.setCoefficients (coeffsBuffer.getReadPointer (0));
            REQUIRE_MESSAGE (filter.getOrder() == firOrder, "Filter order is incorrect!");

            filter.processBlock (actualBuffer);
        }

        const auto* actualData = actualBuffer.getReadPointer (0);
        for (int i = 0; i < blockSize; ++i)
        {
            const auto ref = refBuffer.getSample (0, i);
            REQUIRE_MESSAGE (actualData[i] == Approx (ref).margin (1.0e-3f), "Sample at index: " << std::to_string (i) << " is incorrect!");
        }
    }

    SECTION ("FIR Filter Bypass Test")
    {
        static constexpr double fs = 48000.0;
        static constexpr int blockSize = 512;
        static constexpr int firOrder = 63;

        auto&& coeffsBuffer = test_utils::makeNoise<float> (firOrder);
        auto&& actualBuffer = test_utils::makeSineWave<float> (100.0f, (float) fs, blockSize);
        auto&& refBuffer = juce::AudioBuffer<float> { actualBuffer.getArrayOfWritePointers(), 1, blockSize };

        {
            juce::dsp::FIR::Filter<float> refFilter;
            auto& coefs = refFilter.coefficients->coefficients;
            coefs.resize (firOrder);
            auto* coeffsBufferData = coeffsBuffer.getReadPointer (0);
            std::copy (coeffsBufferData, coeffsBufferData + firOrder, coefs.begin());

            refFilter.prepare ({ fs, (juce::uint32) blockSize, 1 });

            auto&& copyBuffer = juce::AudioBuffer<float> { refBuffer };
            auto&& copyBlock = juce::dsp::AudioBlock<float> { copyBuffer };
            auto&& refBlock = juce::dsp::AudioBlock<float> { refBuffer };
            refFilter.process (juce::dsp::ProcessContextReplacing<float> { copyBlock });
            refFilter.process (juce::dsp::ProcessContextReplacing<float> { refBlock });
        }

        {
            chowdsp::FIRFilter<float> filter;
            filter.setOrder (firOrder);
            filter.setCoefficients (coeffsBuffer.getReadPointer (0));
            REQUIRE_MESSAGE (filter.getOrder() == firOrder, "Filter order is incorrect!");

            auto&& copyBuffer = juce::AudioBuffer<float> { refBuffer };
            auto&& copyBlock = juce::dsp::AudioBlock<float> { copyBuffer };

            filter.processBlock (copyBlock);
            filter.reset();

            filter.processBlockBypassed (actualBuffer);
            filter.processBlock (actualBuffer);
        }

        const auto* actualData = actualBuffer.getReadPointer (0);
        for (int i = 0; i < blockSize; ++i)
        {
            const auto ref = refBuffer.getSample (0, i);
            REQUIRE_MESSAGE (actualData[i] == Approx (ref).margin (1.0e-3f), "Sample at index: " << std::to_string (i) << " is incorrect!");
        }
    }
}
