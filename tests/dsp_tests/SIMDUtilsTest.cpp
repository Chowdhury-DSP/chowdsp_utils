#include <JuceHeader.h>

class SIMDUtilsTest : public UnitTest
{
public:
    SIMDUtilsTest() : UnitTest ("SIMD Utils Test") {}

    template <typename T>
    void testDivide()
    {
        dsp::SIMDRegister<T> six (6.0f);
        dsp::SIMDRegister<T> two (2.0f);

        using namespace chowdsp::SIMDUtils;
        auto three = six / two;

        expect (three.get (0) == (T) 3, "Divide is incorrect!");
    }

    void testClampPi()
    {
        for (float f = -2132.7f; f < 2132.7f; f += 0.741f)
        {
            auto q = chowdsp::SIMDUtils::clampToPiRangeSIMD (dsp::SIMDRegister<float> (f)).get (0);
            expect (q > -MathConstants<float>::pi, "Clamp returned less than negative PI");
            expect (q < MathConstants<float>::pi, "Clamp returned greater than PI");
        }
    }

    template <typename T>
    void testSinCosSIMD (bool clamp, float rangeMultiplier = 1.0f)
    {
        T squareDev = (T) 0, maxDev = (T) 0;
        constexpr int N = 100000;
        for (int i = 0; i < N; ++i)
        {
            dsp::SIMDRegister<T> p ((T) 0);
            dsp::SIMDRegister<T> cp ((T) 0);
            dsp::SIMDRegister<T> sp ((T) 0);
            for (size_t j = 0; j < dsp::SIMDRegister<T>::size(); ++j)
            {
                auto val = MathConstants<T>::twoPi * (T) getRandom().nextFloat() - MathConstants<T>::pi;
                val *= rangeMultiplier;
                p.set (j, val);
                cp.set (j, std::cos (val));
                sp.set (j, std::sin (val));
            }
            p = clamp ? chowdsp::SIMDUtils::clampToPiRangeSIMD (p) : p;

            auto fcp = chowdsp::SIMDUtils::fastcosSIMD (p);
            auto fsp = chowdsp::SIMDUtils::fastsinSIMD (p);

            for (size_t j = 0; j < dsp::SIMDRegister<T>::size(); ++j)
            {
                T cd = std::abs (cp.get (j) - fcp.get (j));
                T sd = std::abs (sp.get (j) - fsp.get (j));
                if (cd > maxDev)
                    maxDev = cd;
                if (sd > maxDev)
                    maxDev = sd;
                squareDev += cd * cd + sd * sd;
            }
        }
        squareDev = std::sqrt (squareDev) / (T) N;
        expect (maxDev < (T) 1e-4, "Max error is too large!");
        expect (squareDev < (T) 1e-6, "Mean squared error is too large!");
    }

    template <typename T>
    void testLoadUnaligned()
    {
        constexpr auto size = dsp::SIMDRegister<T>::size() + 1;
        T testData[size];

        for (size_t i = 0; i < size; ++i)
            testData[i] = (T) i + 1;

        auto reg1 = chowdsp::SIMDUtils::loadUnaligned (testData);
        T sum1 = (T) 0;
        for (size_t i = 0; i < size - 1; ++i)
            sum1 += testData[i];
        expectEquals (sum1, reg1.sum(), "Aligned data is incorrect!");

        auto reg2 = chowdsp::SIMDUtils::loadUnaligned (&testData[1]);
        T sum2 = (T) 0;
        for (size_t i = 1; i < size; ++i)
            sum2 += testData[i];
        expectEquals (sum2, reg2.sum(), "Aligned data is incorrect!");
    }

    template <typename FloatType>
    void baseMathTest (int nIter, std::function<FloatType (FloatType)> floatFunc, std::function<dsp::SIMDRegister<FloatType> (dsp::SIMDRegister<FloatType>)> simdFunc, FloatType maxErr, String functionName)
    {
        for (int i = 0; i < nIter; ++i)
        {
            auto input = (FloatType) Random::getSystemRandom().nextDouble();
            auto expected = floatFunc (input);
            auto actual = simdFunc (dsp::SIMDRegister<FloatType> (input));

            for (size_t j = 0; j < dsp::SIMDRegister<FloatType>::size(); ++j)
                expectWithinAbsoluteError (actual.get (j), expected, maxErr, "SIMD function is incorrect: " + functionName);
        }
    }

    template <typename FloatType>
    void mathTest (int nIter, FloatType maxErr)
    {
        using namespace chowdsp::SIMDUtils;
#define FLOATFUNC(func) [] (FloatType x) { return func (x); }
#define SIMDFUNC(func) [] (dsp::SIMDRegister<FloatType> x) { return func (x); }

        baseMathTest<FloatType> (nIter, FLOATFUNC (std::exp), SIMDFUNC (expSIMD), maxErr, "exp");
        baseMathTest<FloatType> (nIter, FLOATFUNC (std::log), SIMDFUNC (logSIMD), maxErr, "log");
        baseMathTest<FloatType> (nIter, FLOATFUNC (std::sqrt), SIMDFUNC (sqrtSIMD), maxErr, "sqrt");
        baseMathTest<FloatType> (nIter, FLOATFUNC (std::sin), SIMDFUNC (sinSIMD), maxErr, "sin");
        baseMathTest<FloatType> (nIter, FLOATFUNC (std::cos), SIMDFUNC (cosSIMD), maxErr, "cos");
        baseMathTest<FloatType> (nIter, FLOATFUNC (std::tan), SIMDFUNC (tanSIMD), maxErr, "tan");
        baseMathTest<FloatType> (nIter, FLOATFUNC (std::sinh), SIMDFUNC (sinhSIMD), maxErr, "sinh");
        baseMathTest<FloatType> (nIter, FLOATFUNC (std::cosh), SIMDFUNC (coshSIMD), maxErr, "cosh");
        baseMathTest<FloatType> (nIter, FLOATFUNC (std::tanh), SIMDFUNC (tanhSIMD), maxErr, "tanh");

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

    void runTest() override
    {
        beginTest ("Float Divide Test");
        testDivide<float>();

        beginTest ("Double Divide Test");
        testDivide<double>();

        beginTest ("Clamp To Pi Range Test");
        testClampPi();

        beginTest ("Float SIMD sin/cos Test (in range)");
        testSinCosSIMD<float> (false);

        beginTest ("Double SIMD sin/cos Test (in range)");
        testSinCosSIMD<double> (false);

        beginTest ("Float SIMD sin/cos Test (out of range)");
        testSinCosSIMD<float> (true, 10.0f);

        beginTest ("Double SIMD sin/cox Test (out of range)");
        testSinCosSIMD<double> (true, 10.0f);

        beginTest ("Float SIMD sin/cos Test (way out of range)");
        testSinCosSIMD<float> (true, 100.0f);

        beginTest ("Double SIMD sin/cos Test (way out of range)");
        testSinCosSIMD<double> (true, 100.0f);

        beginTest ("Float SIMD load unaligned test");
        testLoadUnaligned<float>();

        beginTest ("Double SIMD load unaligned test");
        testLoadUnaligned<double>();

        beginTest ("Float SIMD Math test");
        mathTest<float> (10, 1.0e-6f);

        beginTest ("Double SIMD Math test");
        mathTest<double> (10, 1.0e-12);
    }
};

static SIMDUtilsTest simdUtilsTest;
