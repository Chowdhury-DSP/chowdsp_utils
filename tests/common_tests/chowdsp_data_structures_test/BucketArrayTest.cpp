#include <CatchUtils.h>
#include <chowdsp_data_structures/chowdsp_data_structures.h>

JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wmissing-field-initializers")

struct TestType
{
    std::string str;
    JUCE_LEAK_DETECTOR (TestType)
};

struct NonTriviallyConstructible
{
    explicit NonTriviallyConstructible (float xx) : x (xx) {}
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NonTriviallyConstructible)
    const float x;
};

TEST_CASE ("Bucket Array Test", "[common][data-structures]")
{
    SECTION ("Add")
    {
        chowdsp::BucketArray<TestType, 4> array;
        array.add (TestType { "abc" });
        array.add (TestType { "def" });
        REQUIRE (array.size() == 2);

        array.add (TestType { "gh" });
        array.add (TestType { "ijk" });
        array.add (TestType { "lmnop" });
        REQUIRE (array.size() == 5);

        TestType numbers { "123" };
        array.add (numbers);
        REQUIRE (array.size() == 6);
    }

    SECTION ("Find by Locator")
    {
        chowdsp::BucketArray<TestType, 4> array;
        [[maybe_unused]] auto [abc_loc, abc] = array.add (TestType { "abc" });
        [[maybe_unused]] auto [def_loc, def] = array.add (TestType { "def" });
        [[maybe_unused]] auto [gh_loc, gh] = array.add (TestType { "gh" });
        [[maybe_unused]] auto [ijk_loc, ijk] = array.add (TestType { "ijk" });
        [[maybe_unused]] auto [lmnop_loc, lmnop] = array.add (TestType { "lmnop" });

        REQUIRE (abc->str == "abc");
        REQUIRE (array.find (abc_loc)->str == "abc");

        REQUIRE (lmnop->str == "lmnop");
        REQUIRE (array.find (lmnop_loc)->str == "lmnop");

        //        REQUIRE (array.find (array.get_locator (nullptr)) == nullptr);
    }

    SECTION ("Get Locator")
    {
        chowdsp::BucketArray<TestType, 4> array;
        const auto [abc_loc, abc] = array.add (TestType { "abc" });
        const auto found_loc = array.get_locator (abc);
        REQUIRE (abc_loc.bucket_index == found_loc.bucket_index);
        REQUIRE (abc_loc.slot_index == found_loc.slot_index);

        const auto invalid_loc = array.get_locator (nullptr);
        REQUIRE (invalid_loc.slot_index == invalid_loc.INVALID_SLOT_INDEX);
    }

    SECTION ("Remove")
    {
        chowdsp::BucketArray<TestType, 4> array;
        [[maybe_unused]] auto [abc_loc, abc] = array.add (TestType { "abc" });
        [[maybe_unused]] auto [def_loc, def] = array.add (TestType { "def" });
        [[maybe_unused]] auto [gh_loc, gh] = array.add (TestType { "gh" });
        [[maybe_unused]] auto [ijk_loc, ijk] = array.add (TestType { "ijk" });
        [[maybe_unused]] auto [lmnop_loc, lmnop] = array.add (TestType { "lmnop" });
        REQUIRE (array.size() == 5);

        array.remove (abc_loc);
        array.remove (lmnop_loc);
        REQUIRE (array.size() == 3);

        [[maybe_unused]] auto [qrs_loc, qrs] = array.add (TestType { "qrs" });
        [[maybe_unused]] auto [tuv_loc, tuv] = array.add (TestType { "tuv" });
        REQUIRE (array.size() == 5);
    }

    SECTION ("Iterate")
    {
        chowdsp::BucketArray<TestType, 4> array;
        array.add (TestType { "abc" });
        array.add (TestType { "def" });
        array.add (TestType { "gh" });
        array.add (TestType { "ijk" });
        array.add (TestType { "lmnop" });

        {
            std::string accum {};
            std::as_const (array).doForAll ([&accum] (const TestType& item)
                                            { accum += item.str; });
            REQUIRE (accum == "abcdefghijklmnop");
        }

        array.doForAll ([] (TestType& item)
                        { item.str.clear(); });
        std::string accum {};
        std::as_const (array).doForAll ([&accum] (const TestType& item)
                                        { accum += item.str; });
        REQUIRE (accum.empty());
    }

    SECTION ("Emplace")
    {
        chowdsp::BucketArray<NonTriviallyConstructible, 2> array;
        [[maybe_unused]] const auto [loc4, elem4] = array.emplace (4.0f);
        [[maybe_unused]] const auto [loc10, elem10] = array.emplace (10.0f);
        REQUIRE (array.size() == 2);
        REQUIRE (juce::exactlyEqual (elem4->x, 4.0f));
    }
}
JUCE_END_IGNORE_WARNINGS_GCC_LIKE
