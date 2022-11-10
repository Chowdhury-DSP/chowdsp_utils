#include <CatchUtils.h>
#include <chowdsp_core/chowdsp_core.h>

TEST_CASE ("String View Helpers Test")
{
    SECTION ("Join")
    {
        static constexpr std::string_view one { "one " };
        static constexpr std::string_view two { "two " };
        static constexpr std::string_view three { "3" };
        static_assert (chowdsp::StringViewHelpers::join_v<one, two, three> == "one two 3");
        REQUIRE (chowdsp::StringViewHelpers::join_v<one, two, three> == "one two 3");
    }
}
