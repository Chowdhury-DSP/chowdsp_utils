#include <CatchUtils.h>
#include <chowdsp_dsp_data_structures/chowdsp_dsp_data_structures.h>

static constexpr float minVal = -10.0f;
static constexpr float maxVal = 10.0f;
static constexpr int N = 1 << 17;
static constexpr int TestN = 1000;

const juce::dsp::LookupTableTransform<float> juceLUT { [] (auto x)
                                                       { return std::tanh (x); },
                                                       minVal,
                                                       maxVal,
                                                       N };
const chowdsp::LookupTableTransform<float> chowLUT { [] (auto x)
                                                     { return std::tanh (x); },
                                                     minVal,
                                                     maxVal,
                                                     N };

TEST_CASE ("Lookup Table Test", "[dsp][data-structures]")
{
    auto minus1To1 = test_utils::RandomFloatGenerator { -1.0f, 1.0f };

    SECTION ("Process Single Checked")
    {
        for (int i = 0; i < TestN; ++i)
        {
            const auto testVal = minus1To1() * 15.0f;
            const auto actualVal = chowLUT (testVal);
            const auto expVal = juceLUT (testVal);
            REQUIRE_MESSAGE (actualVal == Catch::Approx (expVal).margin (1.0e-18f), "Lookup Table value incorrect!");
        }
    }

    SECTION ("Process Single Unchecked")
    {
        for (int i = 0; i < TestN; ++i)
        {
            const auto testVal = minus1To1() * 10.0f;
            const auto actualVal = chowLUT[testVal];
            const auto expVal = juceLUT[testVal];
            REQUIRE_MESSAGE (actualVal == Catch::Approx (expVal).margin (1.0e-18f), "Lookup Table value incorrect!");
        }
    }

    SECTION ("Process Buffer Checked")
    {
        chowdsp::Buffer<float> inBuffer (1, TestN);
        chowdsp::Buffer<float> actualBuffer (1, TestN);
        chowdsp::Buffer<float> expBuffer (1, TestN);
        for (int i = 0; i < TestN; ++i)
            inBuffer.getWritePointer (0)[i] = minus1To1() * 15.0f;

        chowLUT.process (inBuffer.getReadPointer (0), actualBuffer.getWritePointer (0), TestN);
        juceLUT.process (inBuffer.getReadPointer (0), expBuffer.getWritePointer (0), TestN);

        for (int i = 0; i < TestN; ++i)
        {
            const auto actualVal = actualBuffer.getReadPointer (0)[i];
            const auto expVal = expBuffer.getReadPointer (0)[i];
            REQUIRE_MESSAGE (actualVal == Catch::Approx (expVal).margin (1.0e-18f), "Lookup Table value incorrect!");
        }
    }

    SECTION ("Process Buffer Unchecked")
    {
        chowdsp::Buffer<float> inBuffer (1, TestN);
        chowdsp::Buffer<float> actualBuffer (1, TestN);
        chowdsp::Buffer<float> expBuffer (1, TestN);
        for (int i = 0; i < TestN; ++i)
            inBuffer.getWritePointer (0)[i] = minus1To1() * 10.0f;

        chowLUT.processUnchecked (inBuffer.getReadPointer (0), actualBuffer.getWritePointer (0), TestN);
        juceLUT.processUnchecked (inBuffer.getReadPointer (0), expBuffer.getWritePointer (0), TestN);

        for (int i = 0; i < TestN; ++i)
        {
            const auto actualVal = actualBuffer.getReadPointer (0)[i];
            const auto expVal = expBuffer.getReadPointer (0)[i];
            REQUIRE_MESSAGE (actualVal == Catch::Approx (expVal).margin (1.0e-18f), "Lookup Table value incorrect!");
        }
    }
}
