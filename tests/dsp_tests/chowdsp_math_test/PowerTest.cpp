#include <CatchUtils.h>
#include <chowdsp_math/chowdsp_math.h>

using namespace chowdsp::Power;

namespace
{
constexpr int N = 100;
constexpr float maxErrFloat = 1.0e-6f;
constexpr double maxErrDouble = 1.0e-12;
} // namespace

template <int exponent>
void scalarPowerTest()
{
    auto floatRand = test_utils::RandomFloatGenerator { 0.0f, 1.0f };
    auto doubleRand = test_utils::RandomFloatGenerator { 0.0, 1.0 };

    for (int n = 0; n < N; ++n)
    {
        const auto testFloat = floatRand();
        const auto expFloat = std::pow (testFloat, (float) exponent);
        const auto actualFloat = ipow<exponent> (testFloat);
        REQUIRE_MESSAGE (actualFloat == Catch::Approx (expFloat).margin (maxErrFloat), "Float power is incorrect for exponent: " << std::to_string (exponent));

        const auto testDouble = doubleRand();
        const auto expDouble = std::pow (testDouble, (double) exponent);
        const auto actualDouble = ipow<exponent> (testDouble);
        REQUIRE_MESSAGE (actualDouble == Catch::Approx (expDouble).margin (maxErrDouble), "Double power is incorrect for exponent: " << std::to_string (exponent));
    }
}

TEST_CASE ("Power Test", "[dsp][math]")
{
    SECTION ("Scalar Power Test")
    {
        scalarPowerTest<0>();
        scalarPowerTest<1>();
        scalarPowerTest<2>();
        scalarPowerTest<3>();
        scalarPowerTest<4>();
        scalarPowerTest<5>();
        scalarPowerTest<6>();
        scalarPowerTest<7>();
        scalarPowerTest<8>();
        scalarPowerTest<9>();
        scalarPowerTest<10>();
        scalarPowerTest<11>();
        scalarPowerTest<12>();
        scalarPowerTest<13>();
        scalarPowerTest<14>();
        scalarPowerTest<15>();
        scalarPowerTest<16>();
        scalarPowerTest<17>();
        scalarPowerTest<18>();
    }

    SECTION ("Vector Power Test")
    {
        auto floatRand = test_utils::RandomFloatGenerator { 0.0f, 1.0f };
        auto doubleRand = test_utils::RandomFloatGenerator { 0.0, 1.0 };

        for (int exponent = 0; exponent < 19; ++exponent)
        {
            float testFloats[N] {};
            double testDoubles[N] {};

            float expFloats[N] {};
            double expDoubles[N] {};

            float actualFloats[N] {};
            double actualDoubles[N] {};

            for (int n = 0; n < N; ++n)
            {
                testFloats[n] = floatRand();
                expFloats[n] = std::pow (testFloats[n], (float) exponent);

                testDoubles[n] = doubleRand();
                expDoubles[n] = std::pow (testDoubles[n], (double) exponent);
            }

            chowdsp::FloatVectorOperations::integerPower (actualFloats, testFloats, exponent, N);
            chowdsp::FloatVectorOperations::integerPower (actualDoubles, testDoubles, exponent, N);

            for (int n = 0; n < N; ++n)
            {
                REQUIRE_MESSAGE (actualFloats[n] == Catch::Approx (expFloats[n]).margin (maxErrFloat), "Float power is incorrect for exponent: " << std::to_string (exponent));
                REQUIRE_MESSAGE (actualDoubles[n] == Catch::Approx (expDoubles[n]).margin (maxErrDouble), "Double power is incorrect for exponent: " << std::to_string (exponent));
            }
        }
    }
}
