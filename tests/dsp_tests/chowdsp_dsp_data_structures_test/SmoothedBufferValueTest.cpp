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

TEMPLATE_PRODUCT_TEST_CASE ("Smoothed Buffer Value Test", "[dsp][data-structures]", chowdsp::SmoothedBufferValue, ((float, juce::ValueSmoothingTypes::Linear), (double, juce::ValueSmoothingTypes::Multiplicative)))
{
    using FloatType = typename TestType::NumericType;
    using SmoothingType = typename TestType::SmoothingType;
    using OptionalArena = std::optional<std::reference_wrapper<chowdsp::ArenaAllocator<>>>;

    auto testSmooth = [] (auto& ref, auto& comp, FloatType value, int numBlocks, OptionalArena arena = std::nullopt)
    {
        ref.setTargetValue (value);
        for (int i = 0; i < numBlocks; ++i)
        {
            if (arena)
                comp.process (value, maxBlockSize, arena->get());
            else
                comp.process (value, maxBlockSize);
            const auto* smoothData = comp.getSmoothedBuffer();

            for (int n = 0; n < maxBlockSize; ++n)
                REQUIRE_MESSAGE (juce::approximatelyEqual (smoothData[n], ref.getNextValue()), "SmoothedValue was inaccurate!");

            //compare current values of each smoother (once each block)
            REQUIRE_MESSAGE (juce::approximatelyEqual (comp.getCurrentValue(), ref.getCurrentValue()), "Current value is inaccurate!");

            const auto isActuallySmoothing = ! juce::approximatelyEqual (smoothData[0], smoothData[maxBlockSize - 1]);
            REQUIRE_MESSAGE (comp.isSmoothing() == isActuallySmoothing, "SmoothedBufferValue is not smoothing correctly!");
            if (arena)
                arena->get().clear();
        }
    };

    SECTION ("Value Compare Test")
    {
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

        compSmooth.reset ((FloatType) val3);
        refSmooth2.reset (fs, rampLegnth2);
        refSmooth2.setCurrentAndTargetValue ((FloatType) val3);
        testSmooth (refSmooth2, compSmooth, (FloatType) val4, 5);
    }

    SECTION ("Arena Allocator Interface Test")
    {
        chowdsp::ArenaAllocator<> arena { maxBlockSize * sizeof (FloatType) };

        chowdsp::SmoothedBufferValue<FloatType, SmoothingType> arenaSmooth;
        juce::SmoothedValue<FloatType, SmoothingType> refSmooth, refSmooth2;

        arenaSmooth.prepare (fs, maxBlockSize, false);
        arenaSmooth.setRampLength (rampLegnth1);
        refSmooth.reset (fs, rampLegnth1);

        refSmooth.setCurrentAndTargetValue ((FloatType) val1);
        arenaSmooth.reset ((FloatType) val1);
        testSmooth (refSmooth, arenaSmooth, (FloatType) val2, 5, arena);

        arenaSmooth.setRampLength (rampLegnth2);
        refSmooth.reset (fs, rampLegnth2);
        testSmooth (refSmooth, arenaSmooth, (FloatType) val3, 5, arena);

        arenaSmooth.reset ((FloatType) val3);
        refSmooth2.reset (fs, rampLegnth2);
        refSmooth2.setCurrentAndTargetValue ((FloatType) val3);
        testSmooth (refSmooth2, arenaSmooth, (FloatType) val4, 5, arena);
    }
}