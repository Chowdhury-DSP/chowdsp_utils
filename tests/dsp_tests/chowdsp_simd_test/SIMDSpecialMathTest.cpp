#include <TimedUnitTest.h>
#include <chowdsp_simd/chowdsp_simd.h>

using namespace chowdsp::SIMDUtils;

class SIMDSpecialMathTest : public TimedUnitTest
{
public:
    SIMDSpecialMathTest() : TimedUnitTest ("SIMD Special Math Test", "SIMD") {}

    template <typename FloatType>
    void baseMathTest (int nIter, juce::Random& r, std::function<FloatType (FloatType)> floatFunc, std::function<juce::dsp::SIMDRegister<FloatType> (juce::dsp::SIMDRegister<FloatType>)> simdFunc, FloatType maxErr, const juce::String& functionName, const juce::NormalisableRange<FloatType>& range)
    {
        for (int i = 0; i < nIter; ++i)
        {
            auto input = range.convertFrom0to1 ((FloatType) r.nextDouble());
            auto expected = floatFunc (input);
            auto actual = simdFunc (juce::dsp::SIMDRegister<FloatType> (input));

            for (size_t j = 0; j < juce::dsp::SIMDRegister<FloatType>::size(); ++j)
                expectWithinAbsoluteError (actual.get (j), expected, maxErr, "SIMD function is incorrect: " + functionName);
        }
    }

    template <typename FloatType>
    void baseMathTest2D (int nIter, juce::Random& r, std::function<FloatType (FloatType, FloatType)> floatFunc, std::function<juce::dsp::SIMDRegister<FloatType> (juce::dsp::SIMDRegister<FloatType>, juce::dsp::SIMDRegister<FloatType>)> simdFunc, FloatType maxErr, const juce::String& functionName, const juce::NormalisableRange<FloatType>& range)
    {
        for (int i = 0; i < nIter; ++i)
        {
            auto input1 = range.convertFrom0to1 ((FloatType) r.nextDouble());
            auto input2 = range.convertFrom0to1 ((FloatType) r.nextDouble());
            auto expected = floatFunc (input1, input2);
            auto actual = simdFunc (juce::dsp::SIMDRegister<FloatType> (input1), juce::dsp::SIMDRegister<FloatType> (input2));

            for (size_t j = 0; j < juce::dsp::SIMDRegister<FloatType>::size(); ++j)
                expectWithinAbsoluteError (actual.get (j), expected, maxErr, "SIMD function is incorrect: " + functionName);
        }
    }

    template <typename FloatType>
    void mathTest (int nIter, juce::Random& r, FloatType maxErr)
    {
        const auto minus10To10 = juce::NormalisableRange<FloatType> ((FloatType) -10, (FloatType) 10);
        const auto minus1To1 = juce::NormalisableRange<FloatType> ((FloatType) -1, (FloatType) 1);
        const auto sinhRange = juce::NormalisableRange<FloatType> ((FloatType) -3, (FloatType) 3);
        const auto zeroTo10 = juce::NormalisableRange<FloatType> ((FloatType) 0, (FloatType) 10);
        const auto zeroTo2 = juce::NormalisableRange<FloatType> ((FloatType) 0, (FloatType) 2);
        const auto logRange = juce::NormalisableRange<FloatType> ((FloatType) 0.01, (FloatType) 10);

#define FLOATFUNC(func) [] (FloatType x) { return func (x); }
#define SIMDFUNC(func) [] (juce::dsp::SIMDRegister<FloatType> x) { return func (x); }

        baseMathTest<FloatType> (nIter, r, FLOATFUNC (std::exp), SIMDFUNC (expSIMD), maxErr, "exp", sinhRange);
        baseMathTest<FloatType> (nIter, r, FLOATFUNC (std::log), SIMDFUNC (logSIMD), maxErr, "log", logRange);
        baseMathTest<FloatType> (nIter, r, FLOATFUNC (std::log10), SIMDFUNC (log10SIMD), maxErr, "log10", logRange);
        baseMathTest<FloatType> (nIter, r, FLOATFUNC (std::sqrt), SIMDFUNC (sqrtSIMD), maxErr, "sqrt", zeroTo10);
        baseMathTest<FloatType> (nIter, r, FLOATFUNC (std::sin), SIMDFUNC (sinSIMD), maxErr, "sin", minus10To10);
        baseMathTest<FloatType> (nIter, r, FLOATFUNC (std::cos), SIMDFUNC (cosSIMD), maxErr, "cos", minus10To10);
        baseMathTest<FloatType> (nIter, r, FLOATFUNC (std::tan), SIMDFUNC (tanSIMD), maxErr, "tan", minus1To1);
        baseMathTest<FloatType> (nIter, r, FLOATFUNC (std::sinh), SIMDFUNC (sinhSIMD), maxErr, "sinh", sinhRange);
        baseMathTest<FloatType> (nIter, r, FLOATFUNC (std::cosh), SIMDFUNC (coshSIMD), maxErr, "cosh", sinhRange);
        baseMathTest<FloatType> (nIter, r, FLOATFUNC (std::tanh), SIMDFUNC (tanhSIMD), maxErr, "tanh", minus10To10);
        baseMathTest<FloatType> (nIter, r, FLOATFUNC (juce::Decibels::gainToDecibels), SIMDFUNC (gainToDecibels), maxErr, "gainToDecibels", minus10To10);
        baseMathTest<FloatType> (nIter, r, FLOATFUNC (juce::Decibels::decibelsToGain), SIMDFUNC (decibelsToGain), maxErr, "decibelsToGain", minus10To10);

        baseMathTest<FloatType> (
            nIter, r, FLOATFUNC (std::sin), [] (auto x) { return std::get<0> (sincosSIMD (x)); }, maxErr, "sincos", minus10To10);
        baseMathTest<FloatType> (
            nIter, r, FLOATFUNC (std::cos), [] (auto x) { return std::get<1> (sincosSIMD (x)); }, maxErr, "sincos", minus10To10);

#undef FLOATFUNC
#undef SIMDFUNC

#define FLOATFUNC2D(func) [] (FloatType a, FloatType b) { return func (a, b); }
#define SIMDFUNC2D(func) [] (juce::dsp::SIMDRegister<FloatType> a, juce::dsp::SIMDRegister<FloatType> b) { return func (a, b); }

        baseMathTest2D<FloatType> (nIter, r, FLOATFUNC2D (std::pow), SIMDFUNC2D (powSIMD), maxErr * (FloatType) 100, "pow", zeroTo2);
        baseMathTest2D<FloatType> (nIter, r, FLOATFUNC2D (std::atan2), SIMDFUNC2D (atan2SIMD), maxErr, "pow", minus10To10);

#undef FLOATFUNC2D
#undef SIMDFUNC2D
    }

    template <typename FloatType>
    void isnanTest (int nIter, juce::Random& r)
    {
        for (int i = 0; i < nIter; ++i)
        {
            juce::dsp::SIMDRegister<FloatType> vec {};
            int numNans = 0;
            for (size_t j = 0; j < vec.size(); ++j)
            {
                if (r.nextBool())
                {
                    numNans++;
                    vec.set (j, NAN);
                }
                else
                {
                    vec.set (j, (FloatType) r.nextFloat());
                }
            }

            auto isnan = isnanSIMD (vec);
            auto testNumNans = std::abs ((int) isnan.sum());
            expectEquals (testNumNans, numNans, "Incorrect number of NANs detected!");
        }
    }

    template <typename T>
    void hMinMaxTest (int nIter, juce::Random& r)
    {
        auto refMax = [] (const auto& data) { return *std::max_element (data.begin(), data.end()); };

        auto refMin = [] (const auto& data) { return *std::min_element (data.begin(), data.end()); };

        auto refAbsMax = [] (const auto& data) { return std::abs (*std::max_element (data.begin(), data.end(), [] (auto a, auto b) { return std::abs (a) < std::abs (b); })); };

        for (int i = 0; i < nIter; ++i)
        {
            using Vec = juce::dsp::SIMDRegister<T>;
            std::vector<T> vecData (Vec::size(), (T) 0);
            for (auto& v : vecData)
                v = (T) (r.nextFloat() * 2.0f - 1.0f);

            auto vec = loadUnaligned (vecData.data());
            expectEquals (hMinSIMD (vec), refMin (vecData), "Incorrect minimum value!");
            expectEquals (hMaxSIMD (vec), refMax (vecData), "Incorrect maximum value!");
            expectEquals (hAbsMaxSIMD (vec), refAbsMax (vecData), "Incorrect absolute maximum value!");
        }
    }

    void runTestTimed() override
    {
        auto rand = getRandom();

        beginTest ("Float SIMD Math test");
        mathTest<float> (10, rand, 1.0e-5f);

        beginTest ("Double SIMD Math test");
        mathTest<double> (10, rand, 1.0e-12);

        beginTest ("Float SIMD isnan test");
        isnanTest<float> (10, rand);

        beginTest ("Double SIMD isnan test");
        isnanTest<double> (10, rand);

        beginTest ("Float SIMD hmin/hmax test");
        hMinMaxTest<float> (10, rand);

        beginTest ("Double SIMD hmin/hmax test");
        hMinMaxTest<double> (10, rand);
    }
};

static SIMDSpecialMathTest simdSpecialMathTest;
