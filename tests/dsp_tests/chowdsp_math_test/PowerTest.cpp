#include <TimedUnitTest.h>
#include <chowdsp_math/chowdsp_math.h>

using namespace chowdsp::Power;

namespace
{
constexpr int N = 100;
constexpr float maxErrFloat = 1.0e-6f;
constexpr double maxErrDouble = 1.0e-12;
} // namespace

class PowerTest : TimedUnitTest
{
public:
    PowerTest() : TimedUnitTest ("Power Test")
    {
    }

    template <int exponent>
    void scalarPowerTest (juce::Random& r)
    {
        for (int n = 0; n < N; ++n)
        {
            const auto testFloat = r.nextFloat();
            const auto expFloat = std::pow (testFloat, (float) exponent);
            const auto actualFloat = ipow<exponent> (testFloat);
            expectWithinAbsoluteError (actualFloat, expFloat, maxErrFloat, "Float power is incorrect for exponent: " + juce::String (exponent));

            const auto testDouble = r.nextDouble();
            const auto expDouble = std::pow (testDouble, (double) exponent);
            const auto actualDouble = ipow<exponent> (testDouble);
            expectWithinAbsoluteError (actualDouble, expDouble, maxErrDouble, "Double power is incorrect for exponent: " + juce::String (exponent));
        }
    }

    void vectorPowerTest (juce::Random& r, int exponent)
    {
        float testFloats[N] {};
        double testDoubles[N] {};

        float expFloats[N] {};
        double expDoubles[N] {};

        float actualFloats[N] {};
        double actualDoubles[N] {};

        for (int n = 0; n < N; ++n)
        {
            testFloats[n] = r.nextFloat();
            expFloats[n] = std::pow (testFloats[n], (float) exponent);

            testDoubles[n] = r.nextDouble();
            expDoubles[n] = std::pow (testDoubles[n], (double) exponent);
        }

        chowdsp::FloatVectorOperations::integerPower (actualFloats, testFloats, exponent, N);
        chowdsp::FloatVectorOperations::integerPower (actualDoubles, testDoubles, exponent, N);

        for (int n = 0; n < N; ++n)
        {
            expectWithinAbsoluteError (actualFloats[n], expFloats[n], maxErrFloat, "Float power is incorrect for exponent: " + juce::String (exponent));
            expectWithinAbsoluteError (actualDoubles[n], expDoubles[n], maxErrDouble, "Double power is incorrect for exponent: " + juce::String (exponent));
        }
    }

    void runTestTimed() override
    {
        auto&& rand = getRandom();

        beginTest ("Scalar Power Test");
        scalarPowerTest<0> (rand);
        scalarPowerTest<1> (rand);
        scalarPowerTest<2> (rand);
        scalarPowerTest<3> (rand);
        scalarPowerTest<4> (rand);
        scalarPowerTest<5> (rand);
        scalarPowerTest<6> (rand);
        scalarPowerTest<7> (rand);
        scalarPowerTest<8> (rand);
        scalarPowerTest<9> (rand);
        scalarPowerTest<10> (rand);
        scalarPowerTest<11> (rand);
        scalarPowerTest<12> (rand);
        scalarPowerTest<13> (rand);
        scalarPowerTest<14> (rand);
        scalarPowerTest<15> (rand);
        scalarPowerTest<16> (rand);
        scalarPowerTest<17> (rand);
        scalarPowerTest<18> (rand);

        beginTest ("Vector Power Test");
        for (int exp = 0; exp < 19; ++exp)
            vectorPowerTest (rand, exp);
    }
};

static PowerTest powerTest;
