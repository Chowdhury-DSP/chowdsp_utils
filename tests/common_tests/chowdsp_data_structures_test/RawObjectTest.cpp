#include <CatchUtils.h>
#include <chowdsp_data_structures/chowdsp_data_structures.h>

TEST_CASE ("Raw Object Test", "[common][data-structures]")
{
    SECTION ("Forward Declaration")
    {
        struct Test;
        chowdsp::RawObject<Test, 8> object;

        struct Test
        {
            Test (float x, float y) : data ({ x, y }) {}
            std::array<float, 2> data {};
        };
        object.construct (5.0f, 6.0f);
        REQUIRE (juce::exactlyEqual (object.item().data[0], 5.0f));
        REQUIRE (juce::exactlyEqual (object.item().data[1], 6.0f));

        object.item().data[0] = 7.0f;
        REQUIRE (juce::exactlyEqual (std::as_const (object).item().data[0], 7.0f));
    }

    SECTION ("Not Trivially Destructible")
    {
        chowdsp::RawObject<std::string> string;

        string.construct ("blah");
        REQUIRE (string.item().size() == 4);

        string.item() = "blahblah";
        REQUIRE (string.item().size() == 8);

        string.destruct();
        string.construct ("blahblahblah");
        REQUIRE (string.item().size() == 12);

        string.destruct();
    }

    SECTION ("Move Construction")
    {
        const auto get_empty = []
        {
            return chowdsp::RawObject<std::string> {};
        };

        const auto get = []
        {
            chowdsp::RawObject<std::string> str {};
            str.construct ("constructed before move");
            return str;
        };

        JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wpessimizing-move")
        auto string { std::move (get_empty()) };
        JUCE_END_IGNORE_WARNINGS_GCC_LIKE
        string.construct ("constructed after move");
        REQUIRE (string.item().size() == 22);
        string.destruct();

        string = get();
        REQUIRE (string.item().size() == 23);
        string.destruct();
    }

    SECTION ("Move Assignable")
    {
    }
}
