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

    void runTestTimed() override
    {
        beginTest ("Log2 Test");
        log2Test();
    }
};

static OtherMathOpsTest otherMathOpsTest;
