#include <CatchUtils.h>
#include <chowdsp_math/chowdsp_math.h>

using namespace chowdsp::Combinatorics;

TEST_CASE ("Combinatorics Test", "[dsp][math]")
{
    SECTION ("Factorial Test")
    {
        REQUIRE_MESSAGE (factorial (0) == 1, "Factorial 0 is incorrect!");
        REQUIRE_MESSAGE (factorial (1) == 1, "Factorial 1 is incorrect!");
        REQUIRE_MESSAGE (factorial (4) == 24, "Factorial 4 is incorrect!");
        REQUIRE_MESSAGE (factorial (12) == 479001600, "Factorial 12 is incorrect!");
    }

    SECTION ("Permutation Test")
    {
        REQUIRE_MESSAGE (permutation (1, 1) == 1, "1p1 is incorrect!");
        REQUIRE_MESSAGE (permutation (2, 1) == 2, "2p1 is incorrect!");
        REQUIRE_MESSAGE (permutation (2, 2) == 2, "2p2 is incorrect!");
        REQUIRE_MESSAGE (permutation (5, 2) == 20, "5p2 is incorrect!");
        REQUIRE_MESSAGE (permutation (10, 4) == 5040, "10p4 is incorrect!");
    }

    SECTION ("Combination Test")
    {
        REQUIRE_MESSAGE (combination (1, 1) == 1, "1c1 is incorrect!");
        REQUIRE_MESSAGE (combination (2, 1) == 2, "2c1 is incorrect!");
        REQUIRE_MESSAGE (combination (2, 2) == 1, "2c2 is incorrect!");
        REQUIRE_MESSAGE (combination (5, 2) == 10, "5c2 is incorrect!");
        REQUIRE_MESSAGE (combination (10, 4) == 210, "10c4 is incorrect!");
    }
}
