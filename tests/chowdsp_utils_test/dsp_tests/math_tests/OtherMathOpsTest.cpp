#include <TimedUnitTest.h>

class OtherMathOpsTest : public TimedUnitTest
{
public:
    OtherMathOpsTest() : TimedUnitTest ("Other Math Ops Test") {}

    void log2Test()
    {
        expectEquals (chowdsp::log2 (-1), 0, "Negative numbers edge case is not handled correctly!");
        expectEquals (chowdsp::log2 (0), 0, "Zero edge case is not handled correctly!");
        expectEquals (chowdsp::log2 (1), 0, "Log2(1) is incorrect!");
        expectEquals (chowdsp::log2 (63), 6, "Log2(63) is incorrect!");
        expectEquals (chowdsp::log2 (64), 6, "Log2(64) is incorrect!");
        expectEquals (chowdsp::log2 (65), 7, "Log2(65) is incorrect!");
        expectEquals (chowdsp::log2 (127), 7, "Log2(100) is incorrect!");
        expectEquals (chowdsp::log2 (128), 7, "Log2(128) is incorrect!");
    }

    void ceilingDivideTest()
    {
        expectEquals (chowdsp::ceiling_divide (3, 4), 1, "Ceiling divide 3 / 4 should equal 1");
        expectEquals (chowdsp::ceiling_divide (4, 4), 1, "Ceiling divide 4 / 4 should equal 1");
        expectEquals (chowdsp::ceiling_divide (5, 4), 2, "Ceiling divide 5 / 4 should equal 2");
        expectEquals (chowdsp::ceiling_divide (7, 4), 2, "Ceiling divide 7 / 4 should equal 2");
        expectEquals (chowdsp::ceiling_divide (8, 4), 2, "Ceiling divide 8 / 4 should equal 2");
        expectEquals (chowdsp::ceiling_divide (9, 4), 3, "Ceiling divide 9 / 4 should equal 3");
    }

    void runTestTimed() override
    {
        beginTest ("Log2 Test");
        log2Test();

        beginTest ("Ceiling Divide Test");
        ceilingDivideTest();
    }
};

static OtherMathOpsTest otherMathOpsTest;
