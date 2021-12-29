#include <TimedUnitTest.h>

class SIMDSpecialMathTest : public TimedUnitTest
{
public:
    SIMDSpecialMathTest() : TimedUnitTest ("SIMD Special Math Test", "SIMD") {}

    template <typename FloatType>
    void baseMathTest (int nIter, std::function<FloatType (FloatType)> floatFunc, std::function<dsp::SIMDRegister<FloatType> (dsp::SIMDRegister<FloatType>)> simdFunc, FloatType maxErr, const String& functionName, const NormalisableRange<FloatType>& range)
    {
        for (int i = 0; i < nIter; ++i)
        {
            auto input = range.convertFrom0to1 ((FloatType) Random::getSystemRandom().nextDouble());
            auto expected = floatFunc (input);
            auto actual = simdFunc (dsp::SIMDRegister<FloatType> (input));

            for (size_t j = 0; j < dsp::SIMDRegister<FloatType>::size(); ++j)
                expectWithinAbsoluteError (actual.get (j), expected, maxErr, "SIMD function is incorrect: " + functionName);
        }
    }

    template <typename FloatType>
    void mathTest (int nIter, FloatType maxErr)
    {
        const auto minus10To10 = NormalisableRange<FloatType> ((FloatType) -10, (FloatType) 10);
        const auto minus1To1 = NormalisableRange<FloatType> ((FloatType) -1, (FloatType) 1);
        const auto sinhRange = NormalisableRange<FloatType> ((FloatType) -3, (FloatType) 3);
        const auto zeroTo10 = NormalisableRange<FloatType> ((FloatType) 0, (FloatType) 10);
        const auto logRange = NormalisableRange<FloatType> ((FloatType) 0.01, (FloatType) 10);

        using namespace chowdsp::SIMDUtils;
#define FLOATFUNC(func) [] (FloatType x) { return func (x); }
#define SIMDFUNC(func) [] (dsp::SIMDRegister<FloatType> x) { return func (x); }

        baseMathTest<FloatType> (nIter, FLOATFUNC (std::exp), SIMDFUNC (expSIMD), maxErr, "exp", sinhRange);
        baseMathTest<FloatType> (nIter, FLOATFUNC (std::log), SIMDFUNC (logSIMD), maxErr, "log", logRange);
        baseMathTest<FloatType> (nIter, FLOATFUNC (std::log10), SIMDFUNC (log10SIMD), maxErr, "log10", logRange);
        baseMathTest<FloatType> (nIter, FLOATFUNC (std::sqrt), SIMDFUNC (sqrtSIMD), maxErr, "sqrt", zeroTo10);
        baseMathTest<FloatType> (nIter, FLOATFUNC (std::sin), SIMDFUNC (sinSIMD), maxErr, "sin", minus10To10);
        baseMathTest<FloatType> (nIter, FLOATFUNC (std::cos), SIMDFUNC (cosSIMD), maxErr, "cos", minus10To10);
        baseMathTest<FloatType> (nIter, FLOATFUNC (std::tan), SIMDFUNC (tanSIMD), maxErr, "tan", minus1To1);
        baseMathTest<FloatType> (nIter, FLOATFUNC (std::sinh), SIMDFUNC (sinhSIMD), maxErr, "sinh", sinhRange);
        baseMathTest<FloatType> (nIter, FLOATFUNC (std::cosh), SIMDFUNC (coshSIMD), maxErr, "cosh", sinhRange);
        baseMathTest<FloatType> (nIter, FLOATFUNC (std::tanh), SIMDFUNC (tanhSIMD), maxErr, "tanh", minus10To10);
        baseMathTest<FloatType> (nIter, FLOATFUNC (Decibels::gainToDecibels), SIMDFUNC (gainToDecibels), maxErr, "gainToDecibels", minus10To10);
        baseMathTest<FloatType> (nIter, FLOATFUNC (Decibels::decibelsToGain), SIMDFUNC (decibelsToGain), maxErr, "decibelsToGain", minus10To10);

#undef FLOATFUNC
#undef SIMDFUNC

        // test std::pow (needs 2 inputs)
        for (int i = 0; i < nIter; ++i)
        {
            auto a = (FloatType) Random::getSystemRandom().nextDouble();
            auto b = (FloatType) Random::getSystemRandom().nextDouble();
            auto expected = std::pow (a, b);
            auto actual = chowdsp::SIMDUtils::powSIMD (dsp::SIMDRegister<FloatType> (a), dsp::SIMDRegister<FloatType> (b));

            for (size_t j = 0; j < dsp::SIMDRegister<FloatType>::size(); ++j)
                expectWithinAbsoluteError (actual.get (j), expected, maxErr, "SIMD function is incorrect: pow");
        }
    }

    template <typename FloatType>
    void isnanTest (int nIter)
    {
        auto r = Random::getSystemRandom();

        for (int i = 0; i < nIter; ++i)
        {
            dsp::SIMDRegister<FloatType> vec {};
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

            auto isnan = chowdsp::SIMDUtils::isnanSIMD (vec);
            auto testNumNans = std::abs ((int) isnan.sum());
            expectEquals (testNumNans, numNans, "Incorrect number of NANs detected!");
        }
    }

    void runTestTimed() override
    {
        beginTest ("Float SIMD Math test");
        mathTest<float> (10, 1.0e-5f);

        beginTest ("Double SIMD Math test");
        mathTest<double> (10, 1.0e-12);

        beginTest ("Float SIMD isnan test");
        isnanTest<float> (10);

        beginTest ("Double SIMD isnan test");
        isnanTest<double> (10);
    }
};

static SIMDSpecialMathTest simdSpecialMathTest;
