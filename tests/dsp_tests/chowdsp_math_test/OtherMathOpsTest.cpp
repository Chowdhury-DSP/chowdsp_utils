#include <TimedUnitTest.h>
#include <chowdsp_math/chowdsp_math.h>

class OtherMathOpsTest : public TimedUnitTest
{
public:
    OtherMathOpsTest() : TimedUnitTest ("Other Math Ops Test") {}

    void log2Test()
    {
        expectEquals (chowdsp::Math::log2 (-1), 0, "Negative numbers edge case is not handled correctly!");
        expectEquals (chowdsp::Math::log2 (0), 0, "Zero edge case is not handled correctly!");
        expectEquals (chowdsp::Math::log2 (1), 0, "Log2(1) is incorrect!");
        expectEquals (chowdsp::Math::log2 (63), 6, "Log2(63) is incorrect!");
        expectEquals (chowdsp::Math::log2 (64), 6, "Log2(64) is incorrect!");
        expectEquals (chowdsp::Math::log2 (65), 7, "Log2(65) is incorrect!");
        expectEquals (chowdsp::Math::log2 (127), 7, "Log2(100) is incorrect!");
        expectEquals (chowdsp::Math::log2 (128), 7, "Log2(128) is incorrect!");
    }

    void ceilingDivideTest()
    {
        expectEquals (chowdsp::Math::ceiling_divide (3, 4), 1, "Ceiling divide 3 / 4 should equal 1");
        expectEquals (chowdsp::Math::ceiling_divide (4, 4), 1, "Ceiling divide 4 / 4 should equal 1");
        expectEquals (chowdsp::Math::ceiling_divide (5, 4), 2, "Ceiling divide 5 / 4 should equal 2");
        expectEquals (chowdsp::Math::ceiling_divide (7, 4), 2, "Ceiling divide 7 / 4 should equal 2");
        expectEquals (chowdsp::Math::ceiling_divide (8, 4), 2, "Ceiling divide 8 / 4 should equal 2");
        expectEquals (chowdsp::Math::ceiling_divide (9, 4), 3, "Ceiling divide 9 / 4 should equal 3");
    }

    void signumTest()
    {
        expectEquals (chowdsp::Math::sign (-1), -1, "Signum of negative number is incorrect!");
        expectEquals (chowdsp::Math::sign (-10), -1, "Signum of negative number is incorrect!");
        expectEquals (chowdsp::Math::sign (0), 0, "Signum of zero is incorrect!");
        expectEquals (chowdsp::Math::sign (1), 1, "Signum of positive number is incorrect!");
        expectEquals (chowdsp::Math::sign (10), 1, "Signum of positive number is incorrect!");
    }

    void signumSIMDTest()
    {
        xsimd::batch<float> x { -2.0f, 0.0f, 1.0f, 10.0f };
        xsimd::batch<float> expected { -1.0f, 0.0f, 1.0f, 1.0f };
        expect (xsimd::all (chowdsp::Math::sign (x) == expected), "SIMD signum is incorrect");
    }

    void runTestTimed() override
    {
        beginTest ("Log2 Test");
        log2Test();

        beginTest ("Ceiling Divide Test");
        ceilingDivideTest();

        beginTest ("Signum Test");
        signumTest();

        beginTest ("Signum SIMD Test");
        signumSIMDTest();
    }
};

static OtherMathOpsTest otherMathOpsTest;
