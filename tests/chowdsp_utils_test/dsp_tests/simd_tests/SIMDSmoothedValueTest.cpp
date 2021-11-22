#include <JuceHeader.h>

using namespace chowdsp::SIMDUtils;

/** Copied from the JUCE SmoothedValue unit test, and adapted for SIMD types */
template <class FloatType, class SmoothType>
class CommonSmoothedValueTests : public UnitTest
{
public:
    using SmoothedValueType = SIMDSmoothedValue<FloatType, SmoothType>;
    using VecType = dsp::SIMDRegister<FloatType>;

    CommonSmoothedValueTests()
        : UnitTest ("Common Smoothed Value Tests: " + String (std::is_same<FloatType, float>::value ? "Float" : "Double"))
    {
    }

    void runTest() override
    {
        beginTest ("Initial state");
        {
            SmoothedValueType sv;

            auto value = sv.getCurrentValue();
            expectEquals (sv.getTargetValue().get (0), value.get (0));

            sv.getNextValue();
            expectEquals (sv.getCurrentValue().get (0), value.get (0));
            expect (! sv.isSmoothing());
        }

        beginTest ("Resetting");
        {
            auto initialValue = (FloatType) 15.0f;

            SmoothedValueType sv (initialValue);
            sv.reset (3);
            expectEquals (sv.getCurrentValue().get (0), initialValue);

            auto targetValue = initialValue + (FloatType) 1.0f;
            sv.setTargetValue (targetValue);
            expectEquals (sv.getTargetValue().get (0), targetValue);
            expectEquals (sv.getCurrentValue().get (0), initialValue);
            expect (sv.isSmoothing());

            auto currentValue = sv.getNextValue();
            expect (currentValue.get (0) > initialValue);
            expectEquals (sv.getCurrentValue().get (0), currentValue.get (0));
            expectEquals (sv.getTargetValue().get (0), targetValue);
            expect (sv.isSmoothing());

            sv.reset (5);

            expectEquals (sv.getCurrentValue().get (0), targetValue);
            expectEquals (sv.getTargetValue().get (0), targetValue);
            expect (! sv.isSmoothing());

            sv.getNextValue();
            expectEquals (sv.getCurrentValue().get (0), targetValue);

            sv.setTargetValue (1.5f);
            sv.getNextValue();

            auto newStart = (FloatType) 0.2f;
            sv.setCurrentAndTargetValue (newStart);
            expectEquals (sv.getNextValue().get (0), newStart);
            expectEquals (sv.getTargetValue().get (0), newStart);
            expectEquals (sv.getCurrentValue().get (0), newStart);
            expect (! sv.isSmoothing());
        }

        beginTest ("Sample rate");
        {
            SmoothedValueType svSamples { 3.0f };
            auto svTime = svSamples;

            auto numSamples = 12;

            svSamples.reset (numSamples);
            svTime.reset (numSamples * 2, 1.0);

            for (int i = 0; i < numSamples; ++i)
            {
                svTime.skip (1);
                expectWithinAbsoluteError (svSamples.getNextValue().get (0),
                                           svTime.getNextValue().get (0),
                                           (FloatType) 1.0e-7f);
            }
        }

        beginTest ("Skip");
        {
            SmoothedValueType sv;

            sv.reset (12);
            sv.setCurrentAndTargetValue (1.0f);
            sv.setTargetValue (2.0f);

            Array<VecType> reference;

            for (int i = 0; i < 15; ++i)
                reference.add (sv.getNextValue());

            sv.setCurrentAndTargetValue (1.0f);
            sv.setTargetValue (2.0f);

            expectWithinAbsoluteError (sv.skip (1).get (0), reference[0].get (0), (FloatType) 1.0e-6f);
            expectWithinAbsoluteError (sv.skip (1).get (0), reference[1].get (0), (FloatType) 1.0e-6f);
            expectWithinAbsoluteError (sv.skip (2).get (0), reference[3].get (0), (FloatType) 1.0e-6f);
            sv.skip (3);
            expectWithinAbsoluteError (sv.getCurrentValue().get (0), reference[6].get (0), (FloatType) 1.0e-6f);
            expectEquals (sv.skip (300).get (0), sv.getTargetValue().get (0));
            expectEquals (sv.getCurrentValue().get (0), sv.getTargetValue().get (0));
        }

        beginTest ("Negative");
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
                    expect (val.get (0) > start && val.get (0) < end);
                else
                    expect (val.get (0) < start && val.get (0) > end);

                auto nextVal = sv.getNextValue();
                expect (end > start ? (nextVal.get (0) > val.get (0)) : (nextVal.get (0) < val.get (0)));

                auto endVal = sv.skip (500);
                expectEquals (endVal.get (0), end);
                expectEquals (sv.getNextValue().get (0), end);
                expectEquals (sv.getCurrentValue().get (0), end);

                sv.setCurrentAndTargetValue (start);
                sv.setTargetValue (end);

                SmoothedValueType positiveSv { -start };
                positiveSv.reset (numValues);
                positiveSv.setTargetValue (-end);

                for (int i = 0; i < numValues + 2; ++i)
                    expectEquals (sv.getNextValue().get (0), -positiveSv.getNextValue().get (0));
            }
        }
    }
};

static CommonSmoothedValueTests<float, ValueSmoothingTypes::Linear> floatSIMDSmoothTest;
static CommonSmoothedValueTests<double, ValueSmoothingTypes::Multiplicative> doubleSIMDSmoothTest;
