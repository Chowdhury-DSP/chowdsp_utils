#include <chowdsp_simd/chowdsp_simd.h>
using namespace chowdsp::SIMDUtils;

JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wfloat-equal")
#include <CatchUtils.h>

//static CommonSmoothedValueTests<float, juce::ValueSmoothingTypes::Linear> floatSIMDSmoothTest;
//static CommonSmoothedValueTests<double, juce::ValueSmoothingTypes::Multiplicative> doubleSIMDSmoothTest;
TEMPLATE_PRODUCT_TEST_CASE ("Common Smoothed Value Tests", "[dsp][simd]", SIMDSmoothedValue, ((float, juce::ValueSmoothingTypes::Linear), (double, juce::ValueSmoothingTypes::Multiplicative)))
{
    using SmoothedValueType = TestType;
    using VecType = typename SmoothedValueType::VecType;
    using FloatType = typename VecType::value_type;

    SECTION ("Initial state")
    {
        SmoothedValueType sv;

        auto value = sv.getCurrentValue();
        REQUIRE (sv.getTargetValue().get (0) == value.get (0));

        sv.getNextValue();
        REQUIRE (sv.getCurrentValue().get (0) == value.get (0));
        REQUIRE (! sv.isSmoothing());
    }

    SECTION ("Resetting")
    {
        auto initialValue = (FloatType) 15.0f;

        SmoothedValueType sv (initialValue);
        sv.reset (3);
        REQUIRE (sv.getCurrentValue().get (0) == initialValue);

        auto targetValue = initialValue + (FloatType) 1.0f;
        sv.setTargetValue (targetValue);
        REQUIRE (sv.getTargetValue().get (0) == targetValue);
        REQUIRE (sv.getCurrentValue().get (0) == initialValue);
        REQUIRE (sv.isSmoothing());

        auto currentValue = sv.getNextValue();
        REQUIRE (currentValue.get (0) > initialValue);
        REQUIRE (sv.getCurrentValue().get (0) == currentValue.get (0));
        REQUIRE (sv.getTargetValue().get (0) == targetValue);
        REQUIRE (sv.isSmoothing());

        sv.reset (5);

        REQUIRE (sv.getCurrentValue().get (0) == targetValue);
        REQUIRE (sv.getTargetValue().get (0) == targetValue);
        REQUIRE (! sv.isSmoothing());

        sv.getNextValue();
        REQUIRE (sv.getCurrentValue().get (0) == targetValue);

        sv.setTargetValue (1.5f);
        sv.getNextValue();

        auto newStart = (FloatType) 0.2f;
        sv.setCurrentAndTargetValue (newStart);
        REQUIRE (sv.getNextValue().get (0) == newStart);
        REQUIRE (sv.getTargetValue().get (0) == newStart);
        REQUIRE (sv.getCurrentValue().get (0) == newStart);
        REQUIRE (! sv.isSmoothing());
    }

    SECTION ("Sample rate")
    {
        SmoothedValueType svSamples { 3.0f };
        auto svTime = svSamples;

        auto numSamples = 12;

        svSamples.reset (numSamples);
        svTime.reset (numSamples * 2, 1.0);

        for (int i = 0; i < numSamples; ++i)
        {
            svTime.skip (1);
            REQUIRE (svSamples.getNextValue().get (0) == Catch::Approx (svTime.getNextValue().get (0)).margin ((FloatType) 1.0e-7f));
        }
    }

    SECTION ("Skip")
    {
        SmoothedValueType sv;

        sv.reset (12);
        sv.setCurrentAndTargetValue (1.0f);
        sv.setTargetValue (2.0f);

        std::vector<VecType> reference;

        for (int i = 0; i < 15; ++i)
            reference.push_back (sv.getNextValue());

        sv.setCurrentAndTargetValue (1.0f);
        sv.setTargetValue (2.0f);

        REQUIRE (sv.skip (1).get (0) == Catch::Approx (reference[0].get (0)).margin ((FloatType) 1.0e-6f));
        REQUIRE (sv.skip (1).get (0) == Catch::Approx (reference[1].get (0)).margin ((FloatType) 1.0e-6f));
        REQUIRE (sv.skip (2).get (0) == Catch::Approx (reference[3].get (0)).margin ((FloatType) 1.0e-6f));
        sv.skip (3);
        REQUIRE (sv.getCurrentValue().get (0) == Catch::Approx (reference[6].get (0)).margin ((FloatType) 1.0e-6f));
        REQUIRE (sv.skip (300).get (0) == sv.getTargetValue().get (0));
        REQUIRE (sv.getCurrentValue().get (0) == sv.getTargetValue().get (0));
    }

    SECTION ("Negative")
    {
        SmoothedValueType sv;

        auto numValues = 12;
        sv.reset (numValues);

        std::vector<std::pair<FloatType, FloatType>> ranges = { { (FloatType) -1.0f, (FloatType) -2.0f },
                                                                { (FloatType) -100.0f, (FloatType) -3.0f } };

        for (auto range : ranges)
        {
            auto start = range.first, end = range.second;

            sv.setCurrentAndTargetValue (start);
            sv.setTargetValue (end);

            auto val = sv.skip (numValues / 2);

            if (end > start)
            {
                const auto correctOrder = val.get (0) > start && val.get (0) < end;
                REQUIRE (correctOrder);
            }
            else
            {
                const auto correctOrder = val.get (0) < start && val.get (0) > end;
                REQUIRE (correctOrder);
            }

            auto nextVal = sv.getNextValue();
            const auto inRange = end > start ? (nextVal.get (0) > val.get (0)) : (nextVal.get (0) < val.get (0));
            REQUIRE (inRange);

            auto endVal = sv.skip (500);
            REQUIRE (endVal.get (0) == end);
            REQUIRE (sv.getNextValue().get (0) == end);
            REQUIRE (sv.getCurrentValue().get (0) == end);

            sv.setCurrentAndTargetValue (start);
            sv.setTargetValue (end);

            SmoothedValueType positiveSv { -start };
            positiveSv.reset (numValues);
            positiveSv.setTargetValue (-end);

            for (int i = 0; i < numValues + 2; ++i)
                REQUIRE (sv.getNextValue().get (0) == -positiveSv.getNextValue().get (0));
        }
    }
}
JUCE_END_IGNORE_WARNINGS_GCC_LIKE
