#include <TimedUnitTest.h>
#include <chowdsp_simd/chowdsp_simd.h>

using namespace chowdsp::SIMDUtils;

#define FLOATFUNC(func) [] (FloatType x) { return func (x); }
#define SIMDFUNC(func) [] (xsimd::batch<FloatType> x) { return func (x); }

class SIMDSpecialMathTest : public TimedUnitTest
{
public:
    SIMDSpecialMathTest() : TimedUnitTest ("SIMD Special Math Test", "SIMD") {}

    template <typename FloatType>
    void baseMathTest (int nIter, juce::Random& r, std::function<FloatType (FloatType)> floatFunc, std::function<xsimd::batch<FloatType> (xsimd::batch<FloatType>)> simdFunc, FloatType maxErr, const juce::String& functionName, const juce::NormalisableRange<FloatType>& range)
    {
        for (int i = 0; i < nIter; ++i)
        {
            auto input = range.convertFrom0to1 ((FloatType) r.nextDouble());
            auto expected = floatFunc (input);
            auto actual = simdFunc (xsimd::batch<FloatType> (input));

            for (size_t j = 0; j < xsimd::batch<FloatType>::size; ++j)
                expectWithinAbsoluteError (actual.get (j), expected, maxErr, "SIMD function is incorrect: " + functionName);
        }
    }

    template <typename FloatType>
    void decibelsTest (int nIter, juce::Random& r, FloatType maxErr)
    {
        const auto minus10To10 = juce::NormalisableRange<FloatType> ((FloatType) -10, (FloatType) 10);
        baseMathTest<FloatType> (nIter, r, FLOATFUNC (gainToDecibels), SIMDFUNC (gainToDecibels), maxErr, "gainToDecibels", minus10To10);
        baseMathTest<FloatType> (nIter, r, FLOATFUNC (decibelsToGain), SIMDFUNC (decibelsToGain), maxErr, "decibelsToGain", minus10To10);
    }

    template <typename T>
    void hMinMaxTest (int nIter, juce::Random& r)
    {
        auto refMax = [] (const auto& data) { return *std::max_element (data.begin(), data.end()); };

        auto refMin = [] (const auto& data) { return *std::min_element (data.begin(), data.end()); };

        auto refAbsMax = [] (const auto& data) { return std::abs (*std::max_element (data.begin(), data.end(), [] (auto a, auto b) { return std::abs (a) < std::abs (b); })); };

        for (int i = 0; i < nIter; ++i)
        {
            using Vec = xsimd::batch<T>;
            std::vector<T> vecData (Vec::size, (T) 0);
            for (auto& v : vecData)
                v = (T) (r.nextFloat() * 2.0f - 1.0f);

            auto vec = xsimd::load_unaligned (vecData.data());
            expectEquals (hMinSIMD (vec), refMin (vecData), "Incorrect minimum value!");
            expectEquals (hMaxSIMD (vec), refMax (vecData), "Incorrect maximum value!");
            expectEquals (hAbsMaxSIMD (vec), refAbsMax (vecData), "Incorrect absolute maximum value!");
        }
    }

    void runTestTimed() override
    {
        auto rand = getRandom();

        beginTest ("Float SIMD Decibels test");
        decibelsTest<float> (10, rand, 1.0e-5f);

        beginTest ("Double SIMD Decibels test");
        decibelsTest<double> (10, rand, 1.0e-12);

        beginTest ("Float SIMD hmin/hmax test");
        hMinMaxTest<float> (10, rand);

        beginTest ("Double SIMD hmin/hmax test");
        hMinMaxTest<double> (10, rand);
    }
};

#undef FLOATFUNC
#undef SIMDFUNC

static SIMDSpecialMathTest simdSpecialMathTest;
