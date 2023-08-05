#include <CatchUtils.h>
#include <chowdsp_dsp_data_structures/chowdsp_dsp_data_structures.h>

namespace
{
constexpr double fs = 48000.0;
constexpr int maxBlockSize = 512;
constexpr double rampLegnth1 = 0.025;
constexpr float val1 = 0.5;
} // namespace

TEMPLATE_TEST_CASE ("Smoothed Buffer Value Test",
                    "[dsp][data-structures]",
                    (chowdsp::SmoothedBufferValue<float, juce::ValueSmoothingTypes::Linear>),
                    (chowdsp::SmoothedBufferValue<double, juce::ValueSmoothingTypes::Linear>),
                    (chowdsp::SmoothedBufferValue<float, juce::ValueSmoothingTypes::Multiplicative>),
                    (chowdsp::SmoothedBufferValue<double, juce::ValueSmoothingTypes::Multiplicative>) )
{
    using SmoothedValueType = TestType;
    using FloatType = typename SmoothedValueType::NumericType;
    using SmoothingType = typename SmoothedValueType::SmoothingType;

    const auto parameterCompareTest = [] (auto&& mapFunc)
    {
        auto testSmooth = [&mapFunc] (auto& ref, auto& comp, auto& param, FloatType value, int numBlocks)
        {
            ref.setTargetValue (value);
            param.setValueNotifyingHost ((float) value);

            for (int i = 0; i < numBlocks; ++i)
            {
                comp.process (maxBlockSize);
                const auto* smoothData = comp.getSmoothedBuffer();

                for (int n = 0; n < maxBlockSize; ++n)
                    REQUIRE_MESSAGE (juce::approximatelyEqual (smoothData[n], mapFunc (ref.getNextValue())), "SmoothedValue was inaccurate!");
            }

            REQUIRE_MESSAGE (comp.isSmoothing() == ref.isSmoothing(), "SmoothedBufferValue is not smoothing correctly!");
        };

        chowdsp::PercentParameter param { "param", "Name", 0.5f };

        chowdsp::SmoothedBufferValue<FloatType, SmoothingType> compSmooth;
        juce::SmoothedValue<FloatType, SmoothingType> refSmooth;

        compSmooth.setParameterHandle (&param);
        compSmooth.mappingFunction = [&mapFunc] (auto x)
        { return mapFunc (x); };
        compSmooth.prepare (fs, maxBlockSize);
        compSmooth.setRampLength (rampLegnth1);

        refSmooth.reset (fs, rampLegnth1);
        refSmooth.setCurrentAndTargetValue ((FloatType) param.get());

        testSmooth (refSmooth, compSmooth, param, (FloatType) val1, 5);
    };

    SECTION ("Parameter Compare")
    {
        parameterCompareTest ([] (auto x)
                              { return x; });
    }

    SECTION ("Parameter with Mapping")
    {
        parameterCompareTest ([] (auto x)
                              { return std::pow (x, 10.0f); });
    }
}
