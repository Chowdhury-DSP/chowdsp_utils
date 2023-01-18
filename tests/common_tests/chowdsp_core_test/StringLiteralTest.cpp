#include <sstream>
#include <CatchUtils.h>
#include <chowdsp_core/chowdsp_core.h>

TEST_CASE ("String Literal Test", "[common][data-structures]")
{
    SECTION ("Construction")
    {
        static constexpr auto sl1 = chowdsp::StringLiteral { "TEST" };
        STATIC_REQUIRE (sl1.data()[0] == 'T');

        const auto slCopy = sl1;
        REQUIRE (slCopy.size() == 4);

        const auto slMoved = std::move (slCopy);
        REQUIRE (slMoved.size() == 4);
    }

    SECTION ("Equality")
    {
        const auto sl1 = chowdsp::StringLiteral { "TEST" };
        const auto sl2 = chowdsp::StringLiteral { "TES2" };
        REQUIRE (sl1 == "TEST");
        REQUIRE ("TEST" == sl1);
        REQUIRE (sl1 != "TES");
        REQUIRE ("TES" != sl1);
        REQUIRE (sl2 != sl1);

        using namespace std::string_view_literals;
        REQUIRE (sl1 == "TEST"sv);
        REQUIRE ("TEST"sv == sl1);
        REQUIRE (sl1 != "TE"sv);
        REQUIRE ("TE"sv != sl1);
    }

    SECTION ("Concatenation")
    {
        const auto sl1 = chowdsp::StringLiteral { "TEST" };
        const auto sl2 = chowdsp::StringLiteral { "TES2" };
        REQUIRE (sl1 + sl2 == "TESTTES2");
    }

    SECTION ("Stream")
    {
        static constexpr chowdsp::StringLiteral sl { "BLAH" };
        std::stringstream ss;
        ss << sl;
        REQUIRE (ss.str() == (std::string) sl);
    }

    SECTION ("Range-Based For Loop")
    {
        static constexpr chowdsp::StringLiteral sl { "BLAH" };
        int sum = 0;
        for (const auto& ch : sl)
            sum += (int) ch;
        REQUIRE (sum == 279);
    }

    SECTION ("String Conversion")
    {
        static constexpr chowdsp::StringLiteral sl { "BLAH" };
        std::string str = sl;
        REQUIRE (str == "BLAH");
    }

    SECTION ("Concatenation")
    {
        static constexpr chowdsp::StringLiteral sl { "BLAH" };
        const auto sl2 = sl + " BLAH!";
        REQUIRE (sl2 == "BLAH BLAH!");
    }
}
