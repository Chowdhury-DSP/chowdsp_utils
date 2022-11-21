#include <chowdsp_json/chowdsp_json.h>
#include <TimedUnitTest.h>

class JSONTest : public TimedUnitTest
{
public:
    JSONTest() : TimedUnitTest ("JSON Test") {}

    void stringSerializationTest()
    {
        auto testString = juce::String ("TEST_STRING");

        chowdsp::json parent;
        parent["key"] = testString;
        auto returnString = parent["key"].get<juce::String>();

        expectEquals (returnString, testString, "JSON serialized string is incorrect!");
    }

    void jsonFileTest()
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
        expect (jActual == jTest, "JSON returned from file is incorrect!");

        testFile.deleteFile();
    }

    void jsonMemoryBlockTest()
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
        expect (jActual == jTest, "JSON returned from memory block is incorrect!");
    }

    void isSameTypeTest()
    {
        using namespace chowdsp::JSONUtils;

        // number vs. number
        auto num_num = isSameType (chowdsp::json (4), chowdsp::json (2.0f));
        expect (num_num, "Numbers should be the same type!");

        // number vs. string
        auto num_str = isSameType (chowdsp::json (4), chowdsp::json (juce::String ("zzz")));
        expect (! num_str, "Numbers and strings should be different types!");

        // string vs. string
        auto str_str = isSameType (chowdsp::json (juce::String ("aaa")), chowdsp::json ("zzz"));
        expect (str_str, "Strings should be the same type!");
    }

    void binaryDataTest()
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

        expect (jActual == jTest, "JSON returned from binary data is incorrect!");
    }

    void runTestTimed() override
    {
        beginTest ("JUCE String Serialization Test");
        stringSerializationTest();

        beginTest ("JSON File Test");
        jsonFileTest();

        beginTest ("JSON Memory Block Test");
        jsonMemoryBlockTest();

        beginTest ("Binary Data Test");
        binaryDataTest();

        beginTest ("Same Type Test");
        isSameTypeTest();
    }
};

static JSONTest jsonTest;
