#include <CatchUtils.h>
#include <chowdsp_units/chowdsp_units.h>

using namespace chowdsp::Units;

TEST_CASE ("Time Units Test")
{
    SECTION ("Seconds to Seconds")
    {
        constexpr auto tt_seconds = Time<Seconds> { 0.1f };
        constexpr Time<MilliSeconds> tt_milliseconds = tt_seconds;
        static_assert (tt_milliseconds.value() == 100.0f);
        REQUIRE_MESSAGE (tt_milliseconds.value() == 100.0f, tt_milliseconds << " should be equal to " << 100.0f);
    }

    SECTION ("Samples To Seconds (and back)")
    {
        constexpr auto tt_samples = Time<Samples> { 1000.0f, 1000.0f };
        constexpr auto ttt_sec = Time<Seconds> { tt_samples };
        constexpr auto ttt_samples = Time<Samples> { ttt_sec, 500.0f };
        REQUIRE_MESSAGE (ttt_samples.value() == 500.0f, ttt_samples << " should be equal to " << 500.0f);
    }

    SECTION ("Integer Samples")
    {
        constexpr auto tt_samples = Time<SamplesUnit<int>> { 1000, 1000.0 };
        constexpr auto ttt_sec = Time<Seconds> { tt_samples };
        constexpr auto ttt_samples = Time<SamplesUnit<int>> { ttt_sec, 500.0 };
        REQUIRE_MESSAGE (ttt_samples.value() == 500, ttt_samples << " should be equal to " << 500);
    }
}
