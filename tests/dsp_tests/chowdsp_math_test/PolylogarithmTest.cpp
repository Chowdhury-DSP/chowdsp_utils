#include <TimedUnitTest.h>
#include <chowdsp_math/chowdsp_math.h>

class PolylogarithmTest : public TimedUnitTest
{
public:
    PolylogarithmTest() : TimedUnitTest ("Polylogarithm Test") {}

    void Li2Test()
    {
        using chowdsp::Polylogarithm::Li2;
        static constexpr auto maxErr = 1.0e-12;
        expectWithinAbsoluteError (Li2 (-100.0), -12.238755177314939, maxErr, "Li2(-100) is incorrect!");
        expectWithinAbsoluteError (Li2 (-1.0), -0.8224670334241142, maxErr, "Li2(-1) is incorrect!");
        expectWithinAbsoluteError (Li2 (-0.5), -0.4484142069236462, maxErr, "Li2(-0.5) is incorrect!");
        expectWithinAbsoluteError (Li2 (0.0), 0.0, maxErr, "Li2(0) is incorrect!");
        expectWithinAbsoluteError (Li2 (0.25), 0.2676526390827327, maxErr, "Li2(0.25) is incorrect!");
        expectWithinAbsoluteError (Li2 (0.75), 0.978469392930305, maxErr, "Li2(0.75) is incorrect!");
        expectWithinAbsoluteError (Li2 (1.0), 1.6449340668482264, maxErr, "Li2(1) is incorrect!");
        expectWithinAbsoluteError (Li2 (1.5), 2.37439527027248, maxErr, "Li2(1.5) is incorrect!");
        expectWithinAbsoluteError (Li2 (2.5), 2.420790806565934, maxErr, "Li2(2.5) is incorrect!");
        expectWithinAbsoluteError (Li2 (5.5), 1.645957216191436, maxErr, "Li2(5.5) is incorrect!");
    }

    void runTestTimed() override
    {
        beginTest ("Li2 Test");
        Li2Test();
    }
};

static PolylogarithmTest polylogarithmTest;
