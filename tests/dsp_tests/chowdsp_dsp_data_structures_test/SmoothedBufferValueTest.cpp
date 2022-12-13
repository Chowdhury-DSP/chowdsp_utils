#include <CatchUtils.h>
#include <chowdsp_dsp_data_structures/chowdsp_dsp_data_structures.h>

namespace
{
constexpr double fs = 48000.0;
constexpr int maxBlockSize = 512;
constexpr double rampLegnth1 = 0.025;
constexpr double rampLegnth2 = 0.075;

constexpr float val1 = 0.5;
constexpr float val2 = 1.5;
constexpr float val3 = 0.25;
constexpr float val4 = 2.5;
} // namespace

TEMPLATE_PRODUCT_TEST_CASE ("Smoothed Buffer Value Test", "", chowdsp::SmoothedBufferValue, ((float, juce::ValueSmoothingTypes::Linear), (double, juce::ValueSmoothingTypes::Multiplicative)))
{
    using FloatType = typename TestType::NumericType;
    using SmoothingType = typename TestType::SmoothingType;

    SECTION ("Value Compare Test")
    {
        auto testSmooth = [=] (auto& ref, auto& comp, FloatType value, int numBlocks)
        {
            ref.setTargetValue (value);
            for (int i = 0; i < numBlocks; ++i)
            {
                comp.process (value, maxBlockSize);
                const auto* smoothData = comp.getSmoothedBuffer();

                for (int n = 0; n < maxBlockSize; ++n)
                    REQUIRE_MESSAGE (smoothData[n] == ref.getNextValue(), "SmoothedValue was inaccurate!");

                REQUIRE_MESSAGE (comp.getCurrentValue() == ref.getCurrentValue(), "Current value is innacurate!");

                const auto isActuallySmoothing = smoothData[0] != smoothData[maxBlockSize - 1];
                REQUIRE_MESSAGE (comp.isSmoothing() == isActuallySmoothing, "SmoothedBufferValue is not smoothing correctly!");
            }
        };

        chowdsp::SmoothedBufferValue<FloatType, SmoothingType> compSmooth;
        juce::SmoothedValue<FloatType, SmoothingType> refSmooth, refSmooth2;

        compSmooth.prepare (fs, maxBlockSize);
        compSmooth.setRampLength (rampLegnth1);
        refSmooth.reset (fs, rampLegnth1);

        refSmooth.setCurrentAndTargetValue ((FloatType) val1);
        compSmooth.reset ((FloatType) val1);
        testSmooth (refSmooth, compSmooth, (FloatType) val2, 5);

        compSmooth.setRampLength (rampLegnth2);
        refSmooth.reset (fs, rampLegnth2);
        testSmooth (refSmooth, compSmooth, (FloatType) val3, 5);

        compSmooth.reset();
        refSmooth2.reset (fs, rampLegnth2);
        refSmooth2.setCurrentAndTargetValue ((FloatType) val3);
        testSmooth (refSmooth2, compSmooth, (FloatType) val4, 5);
    }
}
