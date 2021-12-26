#include <TimedUnitTest.h>

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
        return std::abs (value * (T) 0.001);
    }

    template <typename T>
    T getMaxErr (std::complex<T> value)
    {
        return std::abs (value * (T) 0.001);
    }

    template <typename T>
    void checkError (T actual, T exp, const String& message)
    {
        auto maxErr = getMaxErr (exp);
        expectWithinAbsoluteError (actual, exp, maxErr, message);
    }

    template <typename T>
    void checkError (std::complex<T> actual, std::complex<T> exp, const String& message)
    {
        auto maxErr = getMaxErr (exp);
        expectWithinAbsoluteError (std::real (actual), std::real (exp), maxErr, message);
        expectWithinAbsoluteError (std::imag (actual), std::imag (exp), maxErr, message);
    }

    template <typename T>
    void checkError (dsp::SIMDRegister<T> actual, dsp::SIMDRegister<T> exp, const String& message)
    {
        auto maxErr = getMaxErr (exp.get (0));
        expectWithinAbsoluteError (actual.get (0), exp.get (0), maxErr, message);
    }

    template <int ORDER, typename Coeffs, typename Args>
    void hornersMethodTest (const Coeffs& coeffs, const Args& args)
    {
        for (auto x : args)
        {
            auto exp = naive<ORDER> (coeffs, x);
            auto actual = horner<ORDER> (coeffs, x);
            checkError (actual, exp, "Incorrect result for order " + String (ORDER));
        }
    }

    template <int ORDER, typename Coeffs, typename Args>
    void estrinsMethodTest (const Coeffs& coeffs, const Args& args)
    {
        for (auto x : args)
        {
            auto exp = naive<ORDER> (coeffs, x);
            auto actual = estrin<ORDER> (coeffs, x);
            checkError (actual, exp, "Incorrect result for order " + String (ORDER));
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

        const dsp::SIMDRegister<float> vfloat4[] = { { 1.0f }, { 0.0f }, { -12.0f }, { 0.5f }, { 3.0f } };
        const dsp::SIMDRegister<double> vdouble7[] = { { 1.0 }, { 0.0 }, { 12.0 }, { -2.0 }, { 0.2 }, { 0.65 }, { 90.0 }, { -121.0 } };
        const std::array<dsp::SIMDRegister<double>, 7> testVDoubleVals { 1.0, -1.0, 0.0, 4.0, -5.5, 100.0, -10000.0 };

        beginTest ("Horner's Method Test SIMD (Float/Double)");
        hornersMethodTest<4> (vfloat4, testFloatVals);
        hornersMethodTest<4> (double4, testVDoubleVals);
        hornersMethodTest<7> (vdouble7, testVDoubleVals);

        beginTest ("Estrin's Method Test SIMD (Float/Double)");
        estrinsMethodTest<4> (vfloat4, testFloatVals);
        estrinsMethodTest<4> (double4, testVDoubleVals);
        estrinsMethodTest<7> (vdouble7, testVDoubleVals);
    }
};

static PolynomialsTest polynomialsTest;
