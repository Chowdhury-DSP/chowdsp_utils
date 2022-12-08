#include <CatchUtils.h>
#include <chowdsp_core/chowdsp_core.h>

TEST_CASE ("String Helpers Test")
{
#if __cplusplus >= 202002L
    SECTION ("String View Concatenation")
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
}
