#include <TimedUnitTest.h>
#include <test_utils.h>

#include <juce_dsp/juce_dsp.h>
#include <chowdsp_filters/chowdsp_filters.h>

class FIRFilterTest : public TimedUnitTest
{
public:
    FIRFilterTest() : TimedUnitTest ("FIR Filter Test", "Filters") {}

    void firFilterTest (juce::Random& rand)
    {
        static constexpr double fs = 48000.0;
        static constexpr int blockSize = 512;
        static constexpr int firOrder = 63;

        auto&& coeffsBuffer = test_utils::makeNoise (rand, firOrder);
        auto&& refBuffer = test_utils::makeSineWave (100.0f, (float) fs, blockSize);
        auto&& actualBuffer = juce::AudioBuffer { refBuffer };

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
            expectEquals (filter.getOrder(), firOrder, "Filter order is incorrect!");

            auto&& block = juce::dsp::AudioBlock<float> { actualBuffer };
            filter.processBlock (block);
        }

        for (int i = 0; i < blockSize; ++i)
        {
            const auto ref = refBuffer.getSample (0, i);
            const auto actual = actualBuffer.getSample (0, i);
            expectWithinAbsoluteError (actual, ref, 1.0e-3f, "Sample at index: " + juce::String (i) + " is incorrect!");
        }
    }

    void firFilterBypassTest (juce::Random& rand)
    {
        static constexpr double fs = 48000.0;
        static constexpr int blockSize = 512;
        static constexpr int firOrder = 63;

        auto&& coeffsBuffer = test_utils::makeNoise (rand, firOrder);
        auto&& refBuffer = test_utils::makeSineWave (100.0f, (float) fs, blockSize);
        auto&& actualBuffer = juce::AudioBuffer { refBuffer };

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
            expectEquals (filter.getOrder(), firOrder, "Filter order is incorrect!");

            auto&& copyBuffer = juce::AudioBuffer<float> { refBuffer };
            auto&& copyBlock = juce::dsp::AudioBlock<float> { copyBuffer };
            auto&& block = juce::dsp::AudioBlock<float> { actualBuffer };

            filter.processBlock (copyBlock);
            filter.reset();

            filter.processBlockBypassed (block);
            filter.processBlock (block);
        }

        for (int i = 0; i < blockSize; ++i)
        {
            const auto ref = refBuffer.getSample (0, i);
            const auto actual = actualBuffer.getSample (0, i);
            expectWithinAbsoluteError (actual, ref, 1.0e-3f, "Sample at index: " + juce::String (i) + " is incorrect!");
        }
    }

    void runTestTimed() override
    {
        auto random = getRandom();

        beginTest ("FIR Filter Test");
        firFilterTest (random);

        beginTest ("FIR Filter Bypass Test");
        firFilterBypassTest (random);
    }
};

static FIRFilterTest firFilterTest;
