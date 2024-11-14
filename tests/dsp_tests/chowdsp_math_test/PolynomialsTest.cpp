#include <CatchUtils.h>
#include <chowdsp_math/chowdsp_math.h>
#include <array>
#include <complex>

using namespace chowdsp::Polynomials;

template <typename T>
T getMaxErr (T value)
{
    using std::abs;
    using xsimd::abs;
    return abs (value * (T) (chowdsp::SampleTypeHelpers::NumericType<T>) 0.001);
}

template <typename T>
T getMaxErr (std::complex<T> value)
{
    return std::abs (value * (T) 0.001);
}

template <typename T>
void checkError (T actual, T exp, const std::string& message)
{
    auto maxErr = getMaxErr (exp);
    REQUIRE_MESSAGE (actual == Catch::Approx (exp).margin (maxErr), message);
}

template <typename T>
void checkError (xsimd::batch<T> actual, xsimd::batch<T> exp, const std::string& message)
{
    T actualData alignas (xsimd::default_arch::alignment())[xsimd::batch<T>::size] {};
    xsimd::store_aligned (actualData, actual);

    T expData alignas (xsimd::default_arch::alignment())[xsimd::batch<T>::size] {};
    xsimd::store_aligned (expData, exp);

    checkError (actualData[0], expData[0], message);
}

template <typename T>
void checkError (std::complex<T> actual, std::complex<T> exp, const std::string& message)
{
    auto maxErr = getMaxErr (exp);
    REQUIRE_MESSAGE (std::real (actual) == Catch::Approx (std::real (exp)).margin (maxErr), message);
    REQUIRE_MESSAGE (std::imag (actual) == Catch::Approx (std::imag (exp)).margin (maxErr), message);
}

template <typename T, typename X, int poly_order>
void hornersMethodTest (const Polynomial<T, poly_order>& poly, nonstd::span<const X> args)
{
    for (auto x : args)
    {
        auto exp = naive<poly_order> (poly, x);
        auto actual = horner<poly_order> (poly, x);
        checkError (actual, exp, "Incorrect result for order " + std::to_string (poly_order));

        auto actual_reverse = horner<poly_order> (poly.template convert<poly_order_ascending>(), x);
        checkError (actual_reverse, exp, "Incorrect result for order " + std::to_string (poly_order));
    }
}

template <typename T, typename X, int poly_order>
void estrinsMethodTest (const Polynomial<T, poly_order>& poly, nonstd::span<const X> args)
{
    for (auto x : args)
    {
        auto exp = naive<poly_order> (poly.template convert<poly_order_ascending>(), x);
        auto actual = estrin<poly_order> (poly, x);
        checkError (actual, exp, "Incorrect result for order " + std::to_string (poly_order));

        auto actual_reverse = estrin<poly_order> (poly.template convert<poly_order_ascending>(), x);
        checkError (actual_reverse, exp, "Incorrect result for order " + std::to_string (poly_order));
    }
}

TEST_CASE ("Polynomials Test", "[dsp][math][simd]")
{
    static constexpr std::array<float, 5> float4 { 1.0f, 0.0f, -12.0f, 0.5f, 0.0f };
    static constexpr std::array<float, 8> float7 { 1.0f, 0.0f, 12.0f, 0.5f, 0.2f, 0.65f, 90.0f, -121.0f };
    static constexpr std::array<double, 5> double4 { 1.0, 0.0, -12.0, 0.5, 0.0 };
    static constexpr std::array<double, 8> double7 { 1.0, 0.0, 12.0, 0.5, 0.2, 0.65, 90.0, -121.0 };

    static constexpr std::array<float, 7> testFloatVals { 1.0f, -1.0f, 0.0f, 4.0f, -5.5f, 100.0f, -10000.0f };
    static constexpr std::array<double, 7> testDoubleVals { 1.0, -1.0, 0.0, 4.0, -5.5, 100.0, -10000.0 };

    SECTION ("Horner's Method Test (Float/Double)")
    {
        hornersMethodTest<float, float, 4> (float4, testFloatVals);
        hornersMethodTest<float, float, 7> (float7, testFloatVals);
        hornersMethodTest<double, double, 4> (double4, testDoubleVals);
        hornersMethodTest<double, double, 7> (double7, testDoubleVals);
    }

    SECTION ("Estrin's Scheme Test (Float/Double)")
    {
        estrinsMethodTest<float, float, 4> (float4, testFloatVals);
        estrinsMethodTest<float, float, 7> (float7, testFloatVals);
        estrinsMethodTest<double, double, 4> (double4, testDoubleVals);
        estrinsMethodTest<double, double, 7> (double7, testDoubleVals);
    }

    static constexpr std::array<std::complex<float>, 5> cfloat4 { 1.0f, 0.0f, { -12.0f, 0.1f }, { 0.5f, -2.0f }, { 0.0f, 3.0f } };
    static constexpr std::array<std::complex<double>, 8> cdouble7 { 1.0, 0.0, { 12.0, 0.1 }, { 0.5, -2.0 }, { 0.2, 0.2 }, { 0.65, -1.1 }, { 90.0, -3.0 }, { -121.0, 23.2 } };
    static constexpr std::array<std::complex<double>, 4> testCDoubleVals { 1.0, { 0.0, -1.0 }, 0.0, { 4.0, 3.0 } };

    SECTION ("Horner's Method Test complex (Float/Double)")
    {
        hornersMethodTest<std::complex<float>, float, 4> (cfloat4, testFloatVals);
        hornersMethodTest<double, std::complex<double>, 4> (double4, testCDoubleVals);
        hornersMethodTest<std::complex<double>, std::complex<double>, 7> (cdouble7, testCDoubleVals);
    }

    SECTION ("Estrin's Method Test complex (Float/Double)")
    {
        estrinsMethodTest<std::complex<float>, float, 4> (cfloat4, testFloatVals);
        estrinsMethodTest<double, std::complex<double>, 4> (double4, testCDoubleVals);
        estrinsMethodTest<std::complex<double>, std::complex<double>, 7> (cdouble7, testCDoubleVals);
    }

    const std::array<xsimd::batch<float>, 5> vfloat4 { xsimd::batch (1.0f), xsimd::batch (0.0f), xsimd::batch (-12.0f), xsimd::batch (0.5f), xsimd::batch (3.0f) };
    const std::array<xsimd::batch<double>, 8> vdouble7 { xsimd::batch (1.0), xsimd::batch (0.0), xsimd::batch (12.0), xsimd::batch (-2.0), xsimd::batch (0.2), xsimd::batch (0.65), xsimd::batch (90.0), xsimd::batch (-121.0) };
    const std::array<xsimd::batch<double>, 7> testVDoubleVals { 1.0, -1.0, 0.0, 4.0, -5.5, 100.0, -10000.0 };

    SECTION ("Horner's Method Test SIMD (Float/Double)")
    {
        hornersMethodTest<xsimd::batch<float>, float, 4> (vfloat4, testFloatVals);
        hornersMethodTest<double, xsimd::batch<double>, 4> (double4, testVDoubleVals);
        hornersMethodTest<xsimd::batch<double>, xsimd::batch<double>, 7> (vdouble7, testVDoubleVals);
    }

    SECTION ("Estrin's Method Test SIMD (Float/Double)")
    {
        estrinsMethodTest<xsimd::batch<float>, float, 4> (vfloat4, testFloatVals);
        estrinsMethodTest<double, xsimd::batch<double>, 4> (double4, testVDoubleVals);
        estrinsMethodTest<xsimd::batch<double>, xsimd::batch<double>, 7> (vdouble7, testVDoubleVals);
    }

    SECTION ("Antiderivative Test")
    {
        {
            static constexpr auto poly = chowdsp::Polynomial<float, 2> { { 1.0f, 2.0f, 1.0f } };
            static constexpr auto test = chowdsp::Polynomials::antiderivative<2> (poly, -1.0f);
            REQUIRE_MESSAGE (juce::approximatelyEqual (test.coeffs[0], 1.0f / 3.0f), "Degree 3 is incorrect!");
            REQUIRE_MESSAGE (juce::approximatelyEqual (test.coeffs[1], 1.0f), "Degree 2 is incorrect!");
            REQUIRE_MESSAGE (juce::approximatelyEqual (test.coeffs[2], 1.0f), "Degree 1 is incorrect!");
            REQUIRE_MESSAGE (juce::approximatelyEqual (test.coeffs[3], -1.0f), "Degree 0 is incorrect!");
        }

        {
            static constexpr auto poly = chowdsp::Polynomial<double, 5, chowdsp::poly_order_ascending> { { -2.5, -3.0, 0.0, 1.0, 2.0, 10.0 } };
            const auto test = chowdsp::Polynomials::antiderivative<5, double> (poly);
            REQUIRE_MESSAGE (juce::approximatelyEqual (test.coeffs[6], 10.0 / 6.0), "Degree 6 is incorrect!");
            REQUIRE_MESSAGE (juce::approximatelyEqual (test.coeffs[5], 2.0 / 5.0), "Degree 5 is incorrect!");
            REQUIRE_MESSAGE (juce::approximatelyEqual (test.coeffs[4], 1.0 / 4.0), "Degree 4 is incorrect!");
            REQUIRE_MESSAGE (juce::approximatelyEqual (test.coeffs[3], 0.0), "Degree 3 is incorrect!");
            REQUIRE_MESSAGE (juce::approximatelyEqual (test.coeffs[2], -3.0 / 2.0), "Degree 2 is incorrect!");
            REQUIRE_MESSAGE (juce::approximatelyEqual (test.coeffs[1], -2.5), "Degree 1 is incorrect!");
            REQUIRE_MESSAGE (juce::approximatelyEqual (test.coeffs[0], 0.0), "Degree 0 is incorrect!");
        }
    }

    SECTION ("Derivative Test")
    {
        {
            static constexpr auto poly = chowdsp::Polynomial<float, 2> { { 1.0f, 2.0f, 1.0f } };
            static constexpr auto test = chowdsp::Polynomials::derivative<2> (poly);
            REQUIRE_MESSAGE (juce::approximatelyEqual (test.coeffs[0], 2.0f), "Degree 1 is incorrect!");
            REQUIRE_MESSAGE (juce::approximatelyEqual (test.coeffs[1], 2.0f), "Degree 0 is incorrect!");
        }

        {
            static constexpr auto poly = chowdsp::Polynomial<double, 5, chowdsp::poly_order_ascending> { { -2.5, -3.0, 0.0, 1.0, 2.0, 10.0 } };
            const auto test = chowdsp::Polynomials::derivative<5, double> (poly);
            REQUIRE_MESSAGE (juce::approximatelyEqual (test.coeffs[4], 50.0), "Degree 4 is incorrect!");
            REQUIRE_MESSAGE (juce::approximatelyEqual (test.coeffs[3], 8.0), "Degree 3 is incorrect!");
            REQUIRE_MESSAGE (juce::approximatelyEqual (test.coeffs[2], 3.0), "Degree 2 is incorrect!");
            REQUIRE_MESSAGE (juce::approximatelyEqual (test.coeffs[1], 0.0), "Degree 1 is incorrect!");
            REQUIRE_MESSAGE (juce::approximatelyEqual (test.coeffs[0], -3.0), "Degree 0 is incorrect!");
        }
    }
}
