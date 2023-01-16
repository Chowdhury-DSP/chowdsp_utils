#include <CatchUtils.h>
#include <chowdsp_version/chowdsp_version.h>

#define CHECK_VERSIONS_CONSTEXPR(strV1, strV2, exp)                                                       \
    {                                                                                                     \
        static constexpr chowdsp::Version v1 { strV1 };                                                   \
        static constexpr chowdsp::Version v2 { strV2 };                                                   \
                                                                                                          \
        static_assert ((v1 == v2) == exp[0], "Incorrect version comparison (equals)!");                   \
        REQUIRE_MESSAGE ((v1 == v2) == exp[0], "Incorrect version comparison (equals)!");                 \
                                                                                                          \
        static_assert ((v1 != v2) == exp[1], "Incorrect version comparison (not equals)!");               \
        REQUIRE_MESSAGE ((v1 != v2) == exp[1], "Incorrect version comparison (not equals)!");             \
                                                                                                          \
        static_assert ((v1 > v2) == exp[2], "Incorrect version comparison (greater than)!");              \
        REQUIRE_MESSAGE ((v1 > v2) == exp[2], "Incorrect version comparison (greater than)!");            \
                                                                                                          \
        static_assert ((v1 < v2) == exp[3], "Incorrect version comparison (less than)!");                 \
        REQUIRE_MESSAGE ((v1 < v2) == exp[3], "Incorrect version comparison (less than)!");               \
                                                                                                          \
        static_assert ((v1 >= v2) == exp[4], "Incorrect version comparison (greater than or equals)!");   \
        REQUIRE_MESSAGE ((v1 >= v2) == exp[4], "Incorrect version comparison (greater than or equals)!"); \
                                                                                                          \
        static_assert ((v1 <= v2) == exp[5], "Incorrect version comparison (less than or equals)!");      \
        REQUIRE_MESSAGE ((v1 <= v2) == exp[5], "Incorrect version comparison (less than or equals)!");    \
    }

TEST_CASE ("Version Test", "[plugin][version]")
{
    SECTION ("Constexpr Version Test")
    {
        using namespace std::string_view_literals;

        static constexpr std::array<bool, 6> exp0 { true, false, false, false, true, true };
        CHECK_VERSIONS_CONSTEXPR ("1.1.1"sv, "1.1.1"sv, exp0);

        static constexpr std::array<bool, 6> exp1 { false, true, true, false, true, false };
        CHECK_VERSIONS_CONSTEXPR ("1.1.2"sv, "1.1.1"sv, exp1);

        static constexpr std::array<bool, 6> exp2 { false, true, false, true, false, true };
        CHECK_VERSIONS_CONSTEXPR ("1.1.0"sv, "1.1.1"sv, exp2);

        static constexpr std::array<bool, 6> exp3 { false, true, true, false, true, false };
        CHECK_VERSIONS_CONSTEXPR ("1.2.1"sv, "1.1.1"sv, exp3);

        static constexpr std::array<bool, 6> exp4 { false, true, false, true, false, true };
        CHECK_VERSIONS_CONSTEXPR ("1.0.1"sv, "1.1.1"sv, exp4);

        static constexpr std::array<bool, 6> exp5 { false, true, true, false, true, false };
        CHECK_VERSIONS_CONSTEXPR ("v2.1.1"sv, "1.1.1"sv, exp5);

        static constexpr std::array<bool, 6> exp6 { false, true, false, true, false, true };
        CHECK_VERSIONS_CONSTEXPR ("0.1.1"sv, "v1.1.1"sv, exp6);

        static constexpr std::array<bool, 6> exp7 { false, true, true, false, true, false };
        CHECK_VERSIONS_CONSTEXPR ("1.1.11"sv, "1.1.9"sv, exp7);

        static constexpr std::array<bool, 6> exp8 { false, true, false, true, false, true };
        CHECK_VERSIONS_CONSTEXPR ("1.9.1"sv, "1.11.1"sv, exp8);
    }

    SECTION ("Version Compare Test")
    {
        auto checkVersions = [] (const juce::String& strV1, const juce::String& strV2, const std::array<bool, 6>& exp)
        {
            const chowdsp::Version v1 (strV1);
            const chowdsp::Version v2 (strV2);

            const chowdsp::Version vv1 ("v" + strV1);
            const chowdsp::Version vv2 ("v" + strV2);

            REQUIRE_MESSAGE ((v1 == v2) == exp[0], "Incorrect version comparison (equals)!");
            REQUIRE_MESSAGE ((vv1 == vv2) == exp[0], "Incorrect version comparison (equals)!");

            REQUIRE_MESSAGE ((v1 != v2) == exp[1], "Incorrect version comparison (not equals)!");
            REQUIRE_MESSAGE ((vv1 != vv2) == exp[1], "Incorrect version comparison (not equals)!");

            REQUIRE_MESSAGE ((v1 > v2) == exp[2], "Incorrect version comparison (greater than)!");
            REQUIRE_MESSAGE ((vv1 > vv2) == exp[2], "Incorrect version comparison (greater than)!");

            REQUIRE_MESSAGE ((v1 < v2) == exp[3], "Incorrect version comparison (less than)!");
            REQUIRE_MESSAGE ((vv1 < vv2) == exp[3], "Incorrect version comparison (less than)!");

            REQUIRE_MESSAGE ((v1 >= v2) == exp[4], "Incorrect version comparison (greater than or equals)!");
            REQUIRE_MESSAGE ((vv1 >= vv2) == exp[4], "Incorrect version comparison (greater than or equals)!");

            REQUIRE_MESSAGE ((v1 <= v2) == exp[5], "Incorrect version comparison (less than or equals)!");
            REQUIRE_MESSAGE ((vv1 <= vv2) == exp[5], "Incorrect version comparison (less than or equals)!");
        };

        checkVersions ("1.1.1", "1.1.1", { true, false, false, false, true, true });
        checkVersions ("1.1.2", "1.1.1", { false, true, true, false, true, false });
        checkVersions ("1.1.0", "1.1.1", { false, true, false, true, false, true });
        checkVersions ("1.2.1", "1.1.1", { false, true, true, false, true, false });
        checkVersions ("1.0.1", "1.1.1", { false, true, false, true, false, true });
        checkVersions ("2.1.1", "1.1.1", { false, true, true, false, true, false });
        checkVersions ("0.1.1", "1.1.1", { false, true, false, true, false, true });
        checkVersions ("1.1.11", "1.1.9", { false, true, true, false, true, false });
        checkVersions ("1.9.1", "1.11.1", { false, true, false, true, false, true });
    }

    SECTION ("Version String Test")
    {
        chowdsp::Version v1 (juce::String { "1.2.3" });
        REQUIRE_MESSAGE (v1.getVersionString() == juce::String ("1.2.3"), "Incorrect version string!");

        chowdsp::Version v2 (juce::String { "v1.2.3" });
        REQUIRE_MESSAGE (v2.getVersionString() == juce::String ("1.2.3"), "Incorrect version string!");
    }

    SECTION ("Version Hint Test")
    {
        using namespace chowdsp::version_literals;
        static constexpr auto v1_2_3 = "1.2.3"_v;
        static_assert (v1_2_3.getVersionHint() == 10203);

        static constexpr auto v50_49_5 = chowdsp::Version { 50, 49, 5 };
        static_assert (v50_49_5.getVersionHint() == 504905);

        const auto v99_99_99 = "99.99.99"_v;
        REQUIRE (v99_99_99.getVersionHint() == 999999);

        const auto v5_49_15 = chowdsp::Version { 5, 49, 15 };
        REQUIRE (v5_49_15.getVersionHint() == 54915);

        REQUIRE (chowdsp::Version {}.getVersionHint() == 0);
    }
}
