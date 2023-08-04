#include <CatchUtils.h>
#include <chowdsp_parameters/chowdsp_parameters.h>

TEST_CASE ("Rhythm Parameter Test", "[plugin][parameters]")
{
    SECTION ("Rhythm Parameter Test")
    {
        auto&& param = chowdsp::RhythmParameter ("rhythm", "Rhythm");
        REQUIRE_MESSAGE (param.getCurrentChoiceName() == juce::String ("1/4"), "Parameter choice label is incorrect!");
        REQUIRE_MESSAGE (juce::approximatelyEqual (param.getRhythmTimeSeconds (60.0), 1.0), "Quarter Note rhythm time is incorrect!");
    }
}
