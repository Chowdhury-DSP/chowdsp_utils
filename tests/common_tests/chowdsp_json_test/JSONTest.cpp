#include <chowdsp_json/chowdsp_json.h>
#include <CatchUtils.h>

TEST_CASE ("JSON Test", "[common][json]")
{
    SECTION ("JUCE String Serialization Test")
    {
        auto testString = juce::String ("TEST_STRING");

        chowdsp::json parent;
        parent["key"] = testString;
        auto returnString = parent["key"].get<juce::String>();

        REQUIRE_MESSAGE (returnString == testString, "JSON serialized string is incorrect!");
    }

    SECTION ("JSON File Test")
    {
        chowdsp::json jTest = {
            { "pi", 3.141 },
            { "happy", true },
            { "name", "Niels" },
            { "nothing", nullptr },
            { "answer", { { "everything", 42 } } },
            { "list", { 1, 0, 2 } },
            { "object", { { "currency", "USD" }, { "value", 42.99 } } }
        };

        const auto testFile = juce::File::getSpecialLocation (juce::File::userHomeDirectory).getChildFile ("test.json");
        chowdsp::JSONUtils::toFile (jTest, testFile, 4); // 4 space indent

        auto jActual = chowdsp::JSONUtils::fromFile (testFile);
        REQUIRE_MESSAGE (jActual == jTest, "JSON returned from file is incorrect!");

        testFile.deleteFile();
    }

    SECTION ("JSON Bad File Test")
    {
        chowdsp::json jTest = {
            { "pi", 3.141 },
            { "happy", true },
            { "name", "Niels" },
            { "nothing", nullptr },
            { "answer", { { "everything", 42 } } },
            { "list", { 1, 0, 2 } },
            { "object", { { "currency", "USD" }, { "value", 42.99 } } }
        };

        const auto testFolder = juce::File::getSpecialLocation (juce::File::userHomeDirectory).getChildFile ("test");
        testFolder.createDirectory();
        testFolder.getChildFile ("test.txt").create();

        chowdsp::JSONUtils::toFile (jTest, testFolder, 4); // 4 space indent
        REQUIRE (testFolder.isDirectory());
        testFolder.deleteRecursively();
    }

    SECTION ("JSON Memory Block Test")
    {
        chowdsp::json jTest = {
            { "pi", 3.141 },
            { "happy", true },
            { "name", "Niels" },
            { "nothing", nullptr },
            { "answer", { { "everything", 42 } } },
            { "list", { 1, 0, 2 } },
            { "object", { { "currency", "USD" }, { "value", 42.99 } } }
        };

        auto testBlock = juce::MemoryBlock {};
        chowdsp::JSONUtils::toMemoryBlock (jTest, testBlock);

        auto jActual = chowdsp::JSONUtils::fromMemoryBlock (testBlock);
        REQUIRE_MESSAGE (jActual == jTest, "JSON returned from memory block is incorrect!");
    }

    SECTION ("Binary Data Test")
    {
        using namespace chowdsp::JSONUtils;

        // number vs. number
        auto num_num = isSameType (chowdsp::json (4), chowdsp::json (2.0f));
        REQUIRE_MESSAGE (num_num, "Numbers should be the same type!");

        // number vs. string
        auto num_str = isSameType (chowdsp::json (4), chowdsp::json (juce::String ("zzz")));
        REQUIRE_MESSAGE (! num_str, "Numbers and strings should be different types!");

        // string vs. string
        auto str_str = isSameType (chowdsp::json (juce::String ("aaa")), chowdsp::json ("zzz"));
        REQUIRE_MESSAGE (str_str, "Strings should be the same type!");
    }

    SECTION ("Same Type Test")
    {
        chowdsp::json jTest = {
            { "pi", 3.141 },
            { "happy", true },
            { "name", "Niels" },
            { "nothing", nullptr },
            { "answer", { { "everything", 42 } } },
            { "list", { 1, 0, 2 } },
            { "object", { { "currency", "USD" }, { "value", 42.99 } } }
        };

        std::string jString = jTest.dump();
        const auto* data = jString.data();
        int size = (int) sizeof (char) * (int) jString.size();
        auto jActual = chowdsp::JSONUtils::fromBinaryData (data, size);

        REQUIRE_MESSAGE (jActual == jTest, "JSON returned from binary data is incorrect!");
    }
}
