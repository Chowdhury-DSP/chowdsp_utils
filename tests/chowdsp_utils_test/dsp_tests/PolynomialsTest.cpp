#include <TimedUnitTest.h>

namespace
{
constexpr std::array<float, 7> testVals { 1.0f, -1.0f, 0.0f, 4.0f, -5.5f, 100.0f, -10000.0f };
} // namespace

using namespace chowdsp::Polynomials;

class PolynomialsTest : TimedUnitTest
{
public:
    PolynomialsTest() : TimedUnitTest ("Polynomials Test")
    {
    }

    template <int ORDER, typename T>
    void hornersMethodTest (const T (&coeffs)[ORDER + 1])
    {
        for (auto x : testVals)
        {
            auto exp = naive<ORDER> (coeffs, (T) x);
            auto actual = horner<ORDER> (coeffs, (T) x);
            auto maxErr = std::abs (exp * (T) 0.001);
            expectWithinAbsoluteError (actual, exp, maxErr, "Incorrect result for order " + String (ORDER));
        }
    }

    template <int ORDER, typename T>
    void estrinsMethodTest (const T (&coeffs)[ORDER + 1])
    {
        for (auto x : testVals)
        {
            auto exp = naive<ORDER> (coeffs, (T) x);
            auto actual = estrin<ORDER> (coeffs, (T) x);
            auto maxErr = std::abs (exp * (T) 0.001);
            expectWithinAbsoluteError (actual, exp, maxErr, "Incorrect result for order " + String (ORDER));
        }
    }

    void runTestTimed() override
    {
        const float float4[] = { 1.0f, 0.0f, -12.0f, 0.5f, 0.0f };
        const float float7[] = { 1.0f, 0.0f, 12.0f, 0.5f, 0.2f, 0.65f, 90.0f, -121.0f };
        const double double4[] = { 1.0, 0.0, -12.0, 0.5, 0.0 };
        const double double7[] = { 1.0, 0.0, 12.0, 0.5, 0.2, 0.65, 90.0, -121.0 };

        beginTest ("Horner's Method Test");
        hornersMethodTest<4> (float4);
        hornersMethodTest<7> (float7);
        hornersMethodTest<4> (double4);
        hornersMethodTest<7> (double7);

        beginTest ("Estrin's Scheme Test");
        estrinsMethodTest<4> (float4);
        estrinsMethodTest<7> (float7);
        estrinsMethodTest<4> (double4);
        estrinsMethodTest<7> (double7);
    }
};

static PolynomialsTest polynomialsTest;
