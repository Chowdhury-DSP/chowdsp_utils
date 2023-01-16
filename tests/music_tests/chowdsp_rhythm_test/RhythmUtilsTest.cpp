#include <CatchUtils.h>
#include <chowdsp_rhythm/chowdsp_rhythm.h>

using namespace chowdsp::RhythmUtils;

TEST_CASE ("Rhythm Utils Test")
{
    SECTION ("Rhythm Time Test")
    {
        STATIC_REQUIRE (QUARTER.getTimeSeconds (60.0) == 1.0);
    }

    SECTION ("Label Test")
    {
        REQUIRE_MESSAGE (QUARTER.getLabel() == "1/4", "Quarter Note label is incorrect!");
    }

    SECTION ("Equality Test")
    {
        STATIC_REQUIRE (QUARTER == QUARTER);
        STATIC_REQUIRE (QUARTER != QUARTER_DOT);
    }
}
