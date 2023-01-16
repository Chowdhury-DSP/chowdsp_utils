#include <CatchUtils.h>
#include <chowdsp_math/chowdsp_math.h>

TEST_CASE ("Polylogarithm Test", "[dsp][math]")
{
    SECTION ("Li2 Test")
    {
        using chowdsp::Polylogarithm::Li2;
        static constexpr auto maxErr = 1.0e-12;
        REQUIRE_MESSAGE (Li2 (-100.0) == Catch::Approx (-12.238755177314939).margin (maxErr), "Li2(-100) is incorrect!");
        REQUIRE_MESSAGE (Li2 (-1.0) == Catch::Approx (-0.8224670334241142).margin (maxErr), "Li2(-1) is incorrect!");
        REQUIRE_MESSAGE (Li2 (-0.5) == Catch::Approx (-0.4484142069236462).margin (maxErr), "Li2(-0.5) is incorrect!");
        REQUIRE_MESSAGE (Li2 (0.0) == Catch::Approx (0.0).margin (maxErr), "Li2(0) is incorrect!");
        REQUIRE_MESSAGE (Li2 (0.25) == Catch::Approx (0.2676526390827327).margin (maxErr), "Li2(0.25) is incorrect!");
        REQUIRE_MESSAGE (Li2 (0.75) == Catch::Approx (0.978469392930305).margin (maxErr), "Li2(0.75) is incorrect!");
        REQUIRE_MESSAGE (Li2 (1.0) == Catch::Approx (1.6449340668482264).margin (maxErr), "Li2(1) is incorrect!");
        REQUIRE_MESSAGE (Li2 (1.5) == Catch::Approx (2.37439527027248).margin (maxErr), "Li2(1.5) is incorrect!");
        REQUIRE_MESSAGE (Li2 (2.5) == Catch::Approx (2.420790806565934).margin (maxErr), "Li2(2.5) is incorrect!");
        REQUIRE_MESSAGE (Li2 (5.5) == Catch::Approx (1.645957216191436).margin (maxErr), "Li2(5.5) is incorrect!");
    }
}
