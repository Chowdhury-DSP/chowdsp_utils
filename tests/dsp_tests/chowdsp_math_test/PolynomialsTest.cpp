#include <TimedUnitTest.h>
#include <chowdsp_math/chowdsp_math.h>

using namespace chowdsp::Polynomials;

class PolynomialsTest : TimedUnitTest
{
public:
    PolynomialsTest() : TimedUnitTest ("Polynomials Test")
    {
    }

    template <typename T>
    T getMaxErr (T value)
    {
        using std::abs;
        using xsimd::abs;
        return abs (value * (T) (NumericType<T>) 0.001);
    }

    template <typename T>
    T getMaxErr (std::complex<T> value)
    {
        return std::abs (value * (T) 0.001);
    }

    template <typename T>
    void checkError (T actual, T exp, const juce::String& message)
    {
        auto maxErr = getMaxErr (exp);
        expectWithinAbsoluteError (actual, exp, maxErr, message);
    }

    template <typename T>
    void checkError (xsimd::batch<T> actual, xsimd::batch<T> exp, const juce::String& message)
    {
        T actualData alignas (xsimd::default_arch::alignment())[xsimd::batch<T>::size] {};
        xsimd::store_aligned (actualData, actual);

        T expData alignas (xsimd::default_arch::alignment())[xsimd::batch<T>::size] {};
        xsimd::store_aligned (expData, exp);

        checkError (actualData[0], expData[0], message);
    }

    template <typename T>
    void checkError (std::complex<T> actual, std::complex<T> exp, const juce::String& message)
    {
        auto maxErr = getMaxErr (exp);
        expectWithinAbsoluteError (std::real (actual), std::real (exp), maxErr, message);
        expectWithinAbsoluteError (std::imag (actual), std::imag (exp), maxErr, message);
    }

    template <int ORDER, typename Coeffs, typename Args>
    void hornersMethodTest (const Coeffs& coeffs, const Args& args)
    {
        for (auto x : args)
        {
            auto exp = naive<ORDER> (coeffs, x);
            auto actual = horner<ORDER> (coeffs, x);
            checkError (actual, exp, "Incorrect result for order " + juce::String (ORDER));
        }
    }

    template <int ORDER, typename Coeffs, typename Args>
    void estrinsMethodTest (const Coeffs& coeffs, const Args& args)
    {
        for (auto x : args)
        {
            auto exp = naive<ORDER> (coeffs, x);
            auto actual = estrin<ORDER> (coeffs, x);
            checkError (actual, exp, "Incorrect result for order " + juce::String (ORDER));
        }
    }

    void antiderivativeTest()
    {
        {
            float testArr[4];
            chowdsp::Polynomials::antiderivative<2> ({ 1.0f, 2.0f, 1.0f }, testArr, -1.0f);
            expectEquals (testArr[0], 1.0f / 3.0f, "Degree 3 is incorrect!");
            expectEquals (testArr[1], 1.0f, "Degree 2 is incorrect!");
            expectEquals (testArr[2], 1.0f, "Degree 1 is incorrect!");
            expectEquals (testArr[3], -1.0f, "Degree 0 is incorrect!");
        }

        {
            double testArr[7];
            chowdsp::Polynomials::antiderivative<5> ({ 10.0, 2.0, 1.0, 0.0, -3.0, -2.5 }, testArr);
            expectEquals (testArr[0], 10.0 / 6.0, "Degree 6 is incorrect!");
            expectEquals (testArr[1], 2.0 / 5.0, "Degree 5 is incorrect!");
            expectEquals (testArr[2], 1.0 / 4.0, "Degree 4 is incorrect!");
            expectEquals (testArr[3], 0.0, "Degree 3 is incorrect!");
            expectEquals (testArr[4], -3.0 / 2.0, "Degree 2 is incorrect!");
            expectEquals (testArr[5], -2.5, "Degree 1 is incorrect!");
            expectEquals (testArr[6], 0.0, "Degree 0 is incorrect!");
        }
    }

    void derivativeTest()
    {
        {
            float testArr[2];
            chowdsp::Polynomials::derivative<2> ({ 1.0f, 2.0f, 1.0f }, testArr);
            expectEquals (testArr[0], 2.0f, "Degree 1 is incorrect!");
            expectEquals (testArr[1], 2.0f, "Degree 0 is incorrect!");
        }

        {
            double testArr[5];
            chowdsp::Polynomials::derivative<5> ({ 10.0, 2.0, 1.0, 0.0, -3.0, -2.5 }, testArr);
            expectEquals (testArr[0], 50.0, "Degree 4 is incorrect!");
            expectEquals (testArr[1], 8.0, "Degree 3 is incorrect!");
            expectEquals (testArr[2], 3.0, "Degree 2 is incorrect!");
            expectEquals (testArr[3], 0.0, "Degree 1 is incorrect!");
            expectEquals (testArr[4], -3.0, "Degree 0 is incorrect!");
        }
    }

    void runTestTimed() override
    {
        const float float4[] = { 1.0f, 0.0f, -12.0f, 0.5f, 0.0f };
        const float float7[] = { 1.0f, 0.0f, 12.0f, 0.5f, 0.2f, 0.65f, 90.0f, -121.0f };
        const double double4[] = { 1.0, 0.0, -12.0, 0.5, 0.0 };
        const double double7[] = { 1.0, 0.0, 12.0, 0.5, 0.2, 0.65, 90.0, -121.0 };

        constexpr std::array<float, 7> testFloatVals { 1.0f, -1.0f, 0.0f, 4.0f, -5.5f, 100.0f, -10000.0f };
        constexpr std::array<double, 7> testDoubleVals { 1.0, -1.0, 0.0, 4.0, -5.5, 100.0, -10000.0 };

        beginTest ("Horner's Method Test (Float/Double)");
        hornersMethodTest<4> (float4, testFloatVals);
        hornersMethodTest<7> (float7, testFloatVals);
        hornersMethodTest<4> (double4, testDoubleVals);
        hornersMethodTest<7> (double7, testDoubleVals);

        beginTest ("Estrin's Scheme Test (Float/Double)");
        estrinsMethodTest<4> (float4, testFloatVals);
        estrinsMethodTest<7> (float7, testFloatVals);
        estrinsMethodTest<4> (double4, testDoubleVals);
        estrinsMethodTest<7> (double7, testDoubleVals);

        const std::complex<float> cfloat4[] = { 1.0f, 0.0f, { -12.0f, 0.1f }, { 0.5f, -2.0f }, { 0.0f, 3.0f } };
        const std::complex<double> cdouble7[] = { 1.0, 0.0, { 12.0, 0.1 }, { 0.5, -2.0 }, { 0.2, 0.2 }, { 0.65, -1.1 }, { 90.0, -3.0 }, { -121.0, 23.2 } };
        constexpr std::array<std::complex<double>, 4> testCDoubleVals { 1.0, { 0.0, -1.0 }, 0.0, { 4.0, 3.0 } };

        beginTest ("Horner's Method Test complex (Float/Double)");
        hornersMethodTest<4> (cfloat4, testFloatVals);
        hornersMethodTest<4> (double4, testCDoubleVals);
        hornersMethodTest<7> (cdouble7, testCDoubleVals);

        beginTest ("Estrin's Method Test complex (Float/Double)");
        estrinsMethodTest<4> (cfloat4, testFloatVals);
        estrinsMethodTest<4> (double4, testCDoubleVals);
        estrinsMethodTest<7> (cdouble7, testCDoubleVals);

        const xsimd::batch<float> vfloat4[] = { xsimd::batch (1.0f), xsimd::batch (0.0f), xsimd::batch (-12.0f), xsimd::batch (0.5f), xsimd::batch (3.0f) };
        const xsimd::batch<double> vdouble7[] = { xsimd::batch (1.0), xsimd::batch (0.0), xsimd::batch (12.0), xsimd::batch (-2.0), xsimd::batch (0.2), xsimd::batch (0.65), xsimd::batch (90.0), xsimd::batch (-121.0) };
        const std::array<xsimd::batch<double>, 7> testVDoubleVals { 1.0, -1.0, 0.0, 4.0, -5.5, 100.0, -10000.0 };

        beginTest ("Horner's Method Test SIMD (Float/Double)");
        hornersMethodTest<4> (vfloat4, testFloatVals);
        hornersMethodTest<4> (double4, testVDoubleVals);
        hornersMethodTest<7> (vdouble7, testVDoubleVals);

        beginTest ("Estrin's Method Test SIMD (Float/Double)");
        estrinsMethodTest<4> (vfloat4, testFloatVals);
        estrinsMethodTest<4> (double4, testVDoubleVals);
        estrinsMethodTest<7> (vdouble7, testVDoubleVals);

        beginTest ("Antiderivative Test");
        antiderivativeTest();

        beginTest ("Derivative Test");
        derivativeTest();
    }
};

static PolynomialsTest polynomialsTest;
