#include <sstream>
#include <CatchUtils.h>
#include <chowdsp_data_structures/chowdsp_data_structures.h>

using namespace chowdsp::string_literals;

TEST_CASE ("String Literal Test", "[common][data-structures]")
{
    SECTION ("Construction")
    {
        static constexpr auto sl1 = chowdsp::StringLiteral { "TEST" };
        STATIC_REQUIRE (sl1.data()[0] == 'T');
        STATIC_REQUIRE (std::is_same_v<std::remove_cv_t<decltype (sl1)>, chowdsp::StringLiteral<5>>);

        const auto slCopy = sl1;
        REQUIRE (slCopy.size() == 4);

        const auto slMoved = std::move (slCopy);
        REQUIRE (slMoved.size() == 4);

        static constexpr auto c1 = chowdsp::StringLiteral<1> { 'a' };
        STATIC_REQUIRE (c1.size() == 1);
        STATIC_REQUIRE (c1.data()[0] == 'a');
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
        using namespace chowdsp::string_literals;
        static constexpr auto sl = "BLAH"_sl;
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

    SECTION ("Numbers")
    {
        static constexpr auto fifteen = chowdsp::StringLiteral<2> { 10 + 5 };
        STATIC_REQUIRE (fifteen == chowdsp::StringLiteral { "15" });

        static constexpr auto m_five = chowdsp::StringLiteral<2> { 5 - 10 };
        STATIC_REQUIRE (m_five == chowdsp::StringLiteral { "-5" });

        static constexpr auto _2048 = chowdsp::StringLiteral<4> { 1 << 11 };
        STATIC_REQUIRE (_2048 == chowdsp::StringLiteral { "2048" });

        // This does not compile (which is correct)!
        // static constexpr auto not_enough_room = chowdsp::StringLiteral<4> { 1'000'000 };
        // REQUIRE (not_enough_room == chowdsp::StringLiteral { "2048" });
    }

    SECTION ("Numberic Literals")
    {
        static constexpr auto fifteen = 15_sl_n;
        REQUIRE (fifteen == chowdsp::StringLiteral { "15" });

        REQUIRE (chowdsp::StringLiteral<2> { (int) 15 } == chowdsp::StringLiteral { "15" });
        REQUIRE (chowdsp::StringLiteral<2> { (size_t) 15 } == chowdsp::StringLiteral { "15" });
        REQUIRE (chowdsp::StringLiteral<3> { (int) -15 } == chowdsp::StringLiteral { "-15" });
    }
}
