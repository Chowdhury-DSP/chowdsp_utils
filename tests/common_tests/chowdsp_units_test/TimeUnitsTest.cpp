#include <CatchUtils.h>

#include <juce_core/juce_core.h>

JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wfloat-equal")
#include <chowdsp_units/chowdsp_units.h>
using namespace chowdsp::Units;

TEST_CASE ("Time Units Test", "[common][units]")
{
    SECTION ("Seconds to Seconds")
    {
        constexpr auto tt_seconds = Time<Seconds> { 0.1f };
        constexpr Time<MilliSeconds> tt_milliseconds = tt_seconds;
        static_assert (tt_milliseconds.value() == 100.0f);
        REQUIRE_MESSAGE (juce::approximatelyEqual (tt_milliseconds.value(), 100.0f), tt_milliseconds << " should be equal to " << 100.0f);
    }

    SECTION ("Samples To Seconds (and back)")
    {
        constexpr auto tt_samples = Time<Samples> { 1000.0f, 1000.0f };
        constexpr auto ttt_sec = Time<Seconds> { tt_samples };
        constexpr auto ttt_samples = Time<Samples> { ttt_sec, 500.0f };
        REQUIRE_MESSAGE (juce::approximatelyEqual (ttt_samples.value(), 500.0f), ttt_samples << " should be equal to " << 500.0f);
    }

    SECTION ("Integer Samples")
    {
        constexpr auto tt_samples = Time<SamplesUnit<int>> { 1000, 1000.0 };
        constexpr auto ttt_sec = Time<Seconds> { tt_samples };
        constexpr auto ttt_samples = Time<SamplesUnit<int>> { ttt_sec, 500.0 };
        REQUIRE_MESSAGE (ttt_samples.value() == 500, ttt_samples << " should be equal to " << 500);
    }

    SECTION ("Comparison Tests")
    {
        constexpr auto tt_seconds = Time<Seconds> { 0.1f };
        constexpr auto tt_milliseconds = Time<MilliSeconds> { 100.0f };
        constexpr auto t_microseconds = Time<MicroSeconds> { 1000.0f };

        // equal/not-equal
        static_assert (tt_seconds == tt_milliseconds);
        REQUIRE (tt_seconds == tt_milliseconds);
        static_assert (tt_seconds != t_microseconds);
        REQUIRE (tt_seconds != t_microseconds);

        // greater than or equal
        static_assert (tt_seconds >= tt_milliseconds);
        REQUIRE (tt_seconds >= tt_milliseconds);
        static_assert (tt_seconds >= t_microseconds);
        REQUIRE (tt_seconds >= t_microseconds);

        // greater than (but not equal)
        static_assert (! (tt_seconds > tt_milliseconds));
        REQUIRE (! (tt_seconds > tt_milliseconds));
        static_assert (tt_seconds > t_microseconds);
        REQUIRE (tt_seconds > t_microseconds);

        // less than or equal
        static_assert (tt_seconds <= tt_milliseconds);
        REQUIRE (tt_seconds <= tt_milliseconds);
        static_assert (t_microseconds <= tt_seconds);
        REQUIRE (t_microseconds <= tt_seconds);

        // less than (but not equal)
        static_assert (! (tt_seconds < tt_milliseconds));
        REQUIRE (! (tt_seconds < tt_milliseconds));
        static_assert (t_microseconds < tt_seconds);
        REQUIRE (t_microseconds < tt_seconds);
    }
}
JUCE_END_IGNORE_WARNINGS_GCC_LIKE
