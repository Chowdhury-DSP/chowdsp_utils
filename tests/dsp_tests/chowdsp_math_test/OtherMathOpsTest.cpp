#include <CatchUtils.h>
#include <chowdsp_math/chowdsp_math.h>

TEST_CASE ("Other Math Ops Test")
{
    SECTION ("constexpr log2 Test")
    {
        static_assert (chowdsp::Math::log2<1>() == 0, "Log2(1) is incorrect!");
        static_assert (chowdsp::Math::log2<2>() == 1, "Log2(2) is incorrect!");
        static_assert (chowdsp::Math::log2<63>() == 6, "Log2(63) is incorrect!");
        static_assert (chowdsp::Math::log2<64>() == 6, "Log2(64) is incorrect!");
        static_assert (chowdsp::Math::log2<65>() == 7, "Log2(65) is incorrect!");
        static_assert (chowdsp::Math::log2<127>() == 7, "Log2(100) is incorrect!");
        static_assert (chowdsp::Math::log2<128>() == 7, "Log2(128) is incorrect!");
    }

    SECTION ("log2 Test")
    {
        REQUIRE_MESSAGE (chowdsp::Math::log2 (-1) == 0, "Negative numbers edge case is not handled correctly!");
        REQUIRE_MESSAGE (chowdsp::Math::log2 (0) == 0, "Zero edge case is not handled correctly!");
        REQUIRE_MESSAGE (chowdsp::Math::log2 (1) == 0, "Log2(1) is incorrect!");
        REQUIRE_MESSAGE (chowdsp::Math::log2 (2) == 1, "Log2(2) is incorrect!");
        REQUIRE_MESSAGE (chowdsp::Math::log2 (63) == 6, "Log2(63) is incorrect!");
        REQUIRE_MESSAGE (chowdsp::Math::log2 (64) == 6, "Log2(64) is incorrect!");
        REQUIRE_MESSAGE (chowdsp::Math::log2 (65) == 7, "Log2(65) is incorrect!");
        REQUIRE_MESSAGE (chowdsp::Math::log2 (127) == 7, "Log2(100) is incorrect!");
        REQUIRE_MESSAGE (chowdsp::Math::log2 (128) == 7, "Log2(128) is incorrect!");
    }

    SECTION ("Ceiling Divide Test")
    {
        REQUIRE_MESSAGE (chowdsp::Math::ceiling_divide (3, 4) == 1, "Ceiling divide 3 / 4 should equal 1");
        REQUIRE_MESSAGE (chowdsp::Math::ceiling_divide (4, 4) == 1, "Ceiling divide 4 / 4 should equal 1");
        REQUIRE_MESSAGE (chowdsp::Math::ceiling_divide (5, 4) == 2, "Ceiling divide 5 / 4 should equal 2");
        REQUIRE_MESSAGE (chowdsp::Math::ceiling_divide (7, 4) == 2, "Ceiling divide 7 / 4 should equal 2");
        REQUIRE_MESSAGE (chowdsp::Math::ceiling_divide (8, 4) == 2, "Ceiling divide 8 / 4 should equal 2");
        REQUIRE_MESSAGE (chowdsp::Math::ceiling_divide (9, 4) == 3, "Ceiling divide 9 / 4 should equal 3");
    }

    SECTION ("Signum Test")
    {
        REQUIRE_MESSAGE (chowdsp::Math::sign (-1) == -1, "Signum of negative number is incorrect!");
        REQUIRE_MESSAGE (chowdsp::Math::sign (-10) == -1, "Signum of negative number is incorrect!");
        REQUIRE_MESSAGE (chowdsp::Math::sign (0) == 0, "Signum of zero is incorrect!");
        REQUIRE_MESSAGE (chowdsp::Math::sign (1) == 1, "Signum of positive number is incorrect!");
        REQUIRE_MESSAGE (chowdsp::Math::sign (10) == 1, "Signum of positive number is incorrect!");
    }

    SECTION ("Signum SIMD Test")
    {
        xsimd::batch<float> x { -2.0f, 0.0f, 1.0f, 10.0f };
        xsimd::batch<float> expected { -1.0f, 0.0f, 1.0f, 1.0f };
        REQUIRE_MESSAGE (xsimd::all (chowdsp::Math::sign (x) == expected), "SIMD signum is incorrect");
    }

    const auto rsqrtTest = [] (auto x_type)
    {
        using FloatType = decltype (x_type);
        using NumericType = chowdsp::SampleTypeHelpers::NumericType<FloatType>;

        for (auto n = (NumericType) -100; n < (NumericType) 100; n += (NumericType) 1)
        {
            const auto x = std::exp2 (n);
            const auto y_exp = FloatType ((NumericType) 1 / std::sqrt (x));
            const auto y_actual = chowdsp::Math::rsqrt ((FloatType) x);

            CHOWDSP_USING_XSIMD_STD (abs);
            const auto percentOff = abs ((y_exp / y_actual) - (NumericType) 1);
            REQUIRE_MESSAGE (percentOff == SIMDApprox<FloatType> (0).margin ((NumericType) 0.005), "rsqrt is inaccurate for input: " + std::to_string (x));
        }
    };

    SECTION ("Rsqrt Test [float]") { rsqrtTest (float {}); }
    SECTION ("Rsqrt Test [double]") { rsqrtTest (double {}); }
    SECTION ("Rsqrt Test [SIMD float]") { rsqrtTest (xsimd::batch<float> {}); }
    SECTION ("Rsqrt Test [SIMD double]") { rsqrtTest (xsimd::batch<double> {}); }

    const auto sigmoidTest = [] (auto x_type)
    {
        using FloatType = decltype (x_type);
        using NumericType = chowdsp::SampleTypeHelpers::NumericType<FloatType>;

        for (auto x = (NumericType) -10; x < (NumericType) 10; x += (NumericType) 0.1)
        {
            const auto y_exp = FloatType (x / (std::sqrt ((NumericType) 1 + std::pow (x, (NumericType) 2))));
            const auto y_actual = chowdsp::Math::algebraicSigmoid ((FloatType) x);
            REQUIRE_MESSAGE (y_actual == SIMDApprox<FloatType> (y_exp).margin ((NumericType) 0.005), "sigmoid is inaccurate for input: " + std::to_string (x));
        }
    };

    SECTION ("Sigmoid Test [float]") { sigmoidTest (float {}); }
    SECTION ("Sigmoid Test [double]") { sigmoidTest (double {}); }
    SECTION ("Sigmoid Test [SIMD float]") { sigmoidTest (xsimd::batch<float> {}); }
    SECTION ("Sigmoid Test [SIMD double]") { sigmoidTest (xsimd::batch<double> {}); }
}
