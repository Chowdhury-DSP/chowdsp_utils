#include <chowdsp_logging/chowdsp_logging.h>
#include <CatchUtils.h>

TEST_CASE ("Custom Formatting Test", "[common][logs]")
{
    chowdsp::ArenaAllocator<> arena { 2048 };

    SECTION ("juce::String")
    {
        juce::String str { "This is a JUCE string!" };
        const auto format_result = chowdsp::format (arena, "{}", str);
        REQUIRE (str == chowdsp::toString (format_result));
    }

    SECTION ("std::span")
    {
        std::vector vec { 0.0f, 1.0f, 2.0f, 3.0f, 4.0f };
        const auto format_result = chowdsp::format (arena, "{}", nonstd::span { vec });
        REQUIRE (format_result == "{0,1,2,3,4}");
    }
}
