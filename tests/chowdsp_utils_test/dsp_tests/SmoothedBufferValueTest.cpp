#include <DummyPlugin.h>

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

template <typename FloatType, typename SmoothingType>
class SmoothedBufferValueTest : public UnitTest
{
public:
    SmoothedBufferValueTest() : UnitTest ("Smoothed Buffer Value Test: " + getSampleType() + " with " + getSmoothingType() + " smoothing")
    {
    }

    static String getSampleType()
    {
        if (std::is_same_v<FloatType, float>)
            return "Float";
        if (std::is_same_v<FloatType, double>)
            return "Double";

        return "Unknown";
    }

    static String getSmoothingType()
    {
        if (std::is_same_v<SmoothingType, ValueSmoothingTypes::Linear>)
            return "Linear";
        if (std::is_same_v<SmoothingType, ValueSmoothingTypes::Multiplicative>)
            return "Multiplicative";

        return "Unknown";
    }

    void valueCompareTest()
    {
        auto testSmooth = [=] (auto& ref, auto& comp, FloatType value, int numBlocks)
        {
            ref.setTargetValue (value);
            for (int i = 0; i < numBlocks; ++i)
            {
                comp.process (value, maxBlockSize);
                const auto* smoothData = comp.getSmoothedBuffer();

                for (int n = 0; n < maxBlockSize; ++n)
                    expectEquals (smoothData[n], ref.getNextValue(), "SmoothedValue was inaccurate!");
            }

            expect (comp.isSmoothing() == ref.isSmoothing(), "SmoothedBufferValue is not smoothing correctly!");
        };

        chowdsp::SmoothedBufferValue<FloatType, SmoothingType> compSmooth;
        SmoothedValue<FloatType, SmoothingType> refSmooth, refSmooth2;

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

    void parameterCompareTest()
    {
        auto testSmooth = [=] (auto& ref, auto& comp, auto* param, FloatType value, int numBlocks)
        {
            ref.setTargetValue (value);
            param->setValueNotifyingHost ((float) value);

            for (int i = 0; i < numBlocks; ++i)
            {
                comp.process (maxBlockSize);
                const auto* smoothData = comp.getSmoothedBuffer();

                for (int n = 0; n < maxBlockSize; ++n)
                    expectEquals (smoothData[n], ref.getNextValue(), "SmoothedValue was inaccurate!");
            }

            expect (comp.isSmoothing() == ref.isSmoothing(), "SmoothedBufferValue is not smoothing correctly!");
        };

        DummyPlugin plugin;
        auto& vts = plugin.getVTS();
        auto* param = vts.getParameter ("dummy");

        chowdsp::SmoothedBufferValue<FloatType, SmoothingType> compSmooth;
        SmoothedValue<FloatType, SmoothingType> refSmooth;

        compSmooth.setParameterHandle (vts.getRawParameterValue ("dummy"));
        compSmooth.prepare (fs, maxBlockSize);
        compSmooth.setRampLength (rampLegnth1);

        refSmooth.reset (fs, rampLegnth1);
        refSmooth.setCurrentAndTargetValue ((FloatType) param->getValue());

        testSmooth (refSmooth, compSmooth, param, (FloatType) val1, 5);
    }

    void runTest() override
    {
        beginTest ("Value Compare Test");
        valueCompareTest();

        beginTest ("Parameter Compare Test");
        parameterCompareTest();
    }
};

static SmoothedBufferValueTest<float, ValueSmoothingTypes::Linear> smoothedBufferValueTest_float_linear;
static SmoothedBufferValueTest<double, ValueSmoothingTypes::Linear> smoothedBufferValueTest_double_linear;
static SmoothedBufferValueTest<float, ValueSmoothingTypes::Multiplicative> smoothedBufferValueTest_float_mult;
static SmoothedBufferValueTest<double, ValueSmoothingTypes::Multiplicative> smoothedBufferValueTest_double_mult;
