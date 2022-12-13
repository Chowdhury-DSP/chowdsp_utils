#include <CatchUtils.h>
#include <chowdsp_core/chowdsp_core.h>

TEST_CASE ("String Helpers Test")
{
#if (defined(__cplusplus) && __cplusplus >= 202002L) || (defined(_MSVC_LANG) && _MSVC_LANG >= 202002L)
    SECTION ("String View Compile-Time Concatenation")
    {
        using namespace std::string_view_literals;
        static constexpr auto sv1 = "blah_"sv;
        static constexpr auto sv2 = "blaaa"sv;
        static constexpr auto sv3 = "_blaa"sv;
        static constexpr auto sv_concat1 = chowdsp::string_view_concat<sv1>;
        static constexpr auto sv_concat2 = chowdsp::string_view_concat<sv1, sv2>;
        static constexpr auto sv_concat3 = chowdsp::string_view_concat<sv1, sv2, sv3>;

        static_assert (sv_concat1 == "blah_"sv, "String view concatenation of one string is incorrect!");
        static_assert (sv_concat2 == "blah_blaaa"sv, "String view concatenation of two string is incorrect!");
        static_assert (sv_concat3 == "blah_blaaa_blaa"sv, "String view concatenation of three string is incorrect!");
    }
#endif

    SECTION ("String View Run-Time Concatenation")
    {
        using namespace std::string_view_literals;
        static constexpr auto sv1 = "blah_"sv;
        static constexpr auto sv2 = "blaaa"sv;
        static constexpr auto sv3 = "_blaa"sv;
        const auto sv_concat1 = chowdsp::concatenate_strings { sv1, sv2 }.value;
        const auto sv_concat2 = chowdsp::concatenate_strings { sv_concat1, sv3 }.value;

        REQUIRE (sv_concat1 == "blah_blaaa"sv);
        REQUIRE (sv_concat2 == "blah_blaaa_blaa"sv);
    }
}
