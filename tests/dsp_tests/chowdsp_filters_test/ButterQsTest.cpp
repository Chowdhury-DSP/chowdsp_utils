#include <CatchUtils.h>
#include <chowdsp_filters/chowdsp_filters.h>

TEST_CASE ("Butterworth Q Values", "[dsp][filters]")
{
    SECTION ("First Order")
    {
        static constexpr auto butterQs = chowdsp::QValCalcs::butterworth_Qs<float, 1>();
        REQUIRE_MESSAGE (butterQs.empty(), "First-Order filter should not have butterworth Qs");
    }

    SECTION ("Second Order")
    {
        static constexpr auto butterQs = chowdsp::QValCalcs::butterworth_Qs<float, 2>();
        REQUIRE_MESSAGE (butterQs.size() == 1, "Second-Order filter should have one Q value");
        REQUIRE (butterQs[0] == Catch::Approx (0.7071f).margin (0.01f));
    }

    SECTION ("Third Order")
    {
        static constexpr auto butterQs = chowdsp::QValCalcs::butterworth_Qs<float, 3>();
        REQUIRE_MESSAGE (butterQs.size() == 1, "Third-Order filter should have one Q value");
        REQUIRE (butterQs[0] == Catch::Approx (1.0f).margin (0.01f));
    }

    SECTION ("Fourth Order")
    {
        static constexpr auto butterQs = chowdsp::QValCalcs::butterworth_Qs<float, 4>();
        REQUIRE_MESSAGE (butterQs.size() == 2, "Fourth-Order filter should have two Q values");
        REQUIRE (butterQs[0] == Catch::Approx (1.3065f).margin (0.01f));
        REQUIRE (butterQs[1] == Catch::Approx (0.5412f).margin (0.01f));
    }

    SECTION ("Fifth Order")
    {
        static constexpr auto butterQs = chowdsp::QValCalcs::butterworth_Qs<float, 5>();
        REQUIRE_MESSAGE (butterQs.size() == 2, "Fifth-Order filter should have two Q values");
        REQUIRE (butterQs[0] == Catch::Approx (1.6181f).margin (0.01f));
        REQUIRE (butterQs[1] == Catch::Approx (0.6180f).margin (0.01f));
    }

    SECTION ("Sixth Order")
    {
        static constexpr auto butterQs = chowdsp::QValCalcs::butterworth_Qs<float, 6>();
        REQUIRE_MESSAGE (butterQs.size() == 3, "Sixth-Order filter should have three Q values");
        REQUIRE (butterQs[0] == Catch::Approx (1.9320f).margin (0.01f));
        REQUIRE (butterQs[1] == Catch::Approx (0.7071f).margin (0.01f));
        REQUIRE (butterQs[2] == Catch::Approx (0.5177f).margin (0.01f));
    }

    SECTION ("Seventh Order")
    {
        static constexpr auto butterQs = chowdsp::QValCalcs::butterworth_Qs<float, 7>();
        REQUIRE_MESSAGE (butterQs.size() == 3, "Seventh-Order filter should have three Q values");
        REQUIRE (butterQs[0] == Catch::Approx (2.2472f).margin (0.01f));
        REQUIRE (butterQs[1] == Catch::Approx (0.8019f).margin (0.01f));
        REQUIRE (butterQs[2] == Catch::Approx (0.5549f).margin (0.01f));
    }

    SECTION ("Eighth Order")
    {
        static constexpr auto butterQs = chowdsp::QValCalcs::butterworth_Qs<float, 8>();
        REQUIRE_MESSAGE (butterQs.size() == 4, "Eighth-Order filter should have four Q values");
        REQUIRE (butterQs[0] == Catch::Approx (2.5628f).margin (0.01f));
        REQUIRE (butterQs[1] == Catch::Approx (0.8999f).margin (0.01f));
        REQUIRE (butterQs[2] == Catch::Approx (0.6013f).margin (0.01f));
        REQUIRE (butterQs[3] == Catch::Approx (0.5098f).margin (0.01f));
    }
}
