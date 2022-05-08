#include <TimedUnitTest.h>
#include <chowdsp_math/chowdsp_math.h>

using namespace chowdsp::Combinatorics;

class CombinatoricsTest : public TimedUnitTest
{
public:
    CombinatoricsTest() : TimedUnitTest ("Combinatorics Test") {}

    void runTestTimed() override
    {
        beginTest ("Factorial Test");
        expectEquals (factorial (0), 1, "Factorial 0 is incorrect!");
        expectEquals (factorial (1), 1, "Factorial 1 is incorrect!");
        expectEquals (factorial (4), 24, "Factorial 4 is incorrect!");
        expectEquals (factorial (12), 479001600, "Factorial 12 is incorrect!");

        beginTest ("Permutation Test");
        expectEquals (permutation (1, 1), 1, "1p1 is incorrect!");
        expectEquals (permutation (2, 1), 2, "2p1 is incorrect!");
        expectEquals (permutation (2, 2), 2, "2p2 is incorrect!");
        expectEquals (permutation (5, 2), 20, "5p2 is incorrect!");
        expectEquals (permutation (10, 4), 5040, "10p4 is incorrect!");

        beginTest ("Combination Test");
        expectEquals (combination (1, 1), 1, "1c1 is incorrect!");
        expectEquals (combination (2, 1), 2, "2c1 is incorrect!");
        expectEquals (combination (2, 2), 1, "2c2 is incorrect!");
        expectEquals (combination (5, 2), 10, "5c2 is incorrect!");
        expectEquals (combination (10, 4), 210, "10c4 is incorrect!");
    }
};

static CombinatoricsTest combinatoricsTest;
