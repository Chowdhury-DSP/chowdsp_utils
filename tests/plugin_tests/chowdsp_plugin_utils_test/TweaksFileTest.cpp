#include <CatchUtils.h>
#include <chowdsp_plugin_utils/chowdsp_plugin_utils.h>

namespace
{
const juce::File testTweaksFile = juce::File::getSpecialLocation (juce::File::SpecialLocationType::userDesktopDirectory)
                                      .getChildFile ("chowdsp_utils_test_config.json");

constexpr std::string_view randomChars { "0123456789 ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz" };

juce::String getRandomString()
{
    test_utils::RandomIntGenerator randLength { 10, 50 };
    test_utils::RandomIntGenerator randChar { 0, (int) randomChars.length() - 1 };

    const auto stringLen = randLength();
    juce::String str;
    for (int i = 0; i < stringLen; ++i)
    {
        const auto nextChar = randomChars[(size_t) randChar()];
        str.append (juce::String::charToString (nextChar), 1);
    }

    return str;
}

namespace BinaryData
{
    const unsigned char temp_binary_data_0[] =
        "{"
        "  \"test_int\": 44,"
        "  \"test_string\": \"blah blah\","
        "  \"test_float\": 42"
        "}\n";

    const char* test_tweaks_file = (const char*) temp_binary_data_0;

    const int test_tweaks_fileSize = 68;
} // namespace BinaryData
} // namespace

TEST_CASE ("Tweaks File Test", "[plugin][utilities]")
{
    SECTION ("Write/Read Test")
    {
        chowdsp::GenericTweaksFile<false> tweaksFile;
        tweaksFile.initialise (testTweaksFile, 1);

        tweaksFile.addProperties ({ { "test_int", 0 }, { "test_string", juce::String {} } });
        tweaksFile.getProperty ("test_float", -1.0f);

        REQUIRE_MESSAGE (tweaksFile.getProperty<int> ("test_int") == 0, "Initial integer property is incorrect");
        REQUIRE_MESSAGE (tweaksFile.getProperty<juce::String> ("test_string") == juce::String {}, "Initial string property is incorrect");
        REQUIRE_MESSAGE (juce::approximatelyEqual (tweaksFile.getProperty<float> ("test_float"), -1.0f), "Initial float property is incorrect");

        test_utils::RandomIntGenerator randInt { -100, 100 };
        test_utils::RandomFloatGenerator randFloat { 0.0f, 1.0f };
        for (int i = 0; i < 10; ++i)
        {
            const auto newInt = randInt();
            const auto newStr = getRandomString();
            const auto newFloat = randFloat();

            const auto jsonConfig = chowdsp::json {
                { "test_int", newInt },
                { "test_string", newStr },
                { "test_float", newFloat },
            };
            chowdsp::JSONUtils::toFile (jsonConfig, testTweaksFile);

            juce::MessageManager::getInstance()->runDispatchLoopUntil (2000);

            REQUIRE_MESSAGE (tweaksFile.getProperty<int> ("test_int") == newInt, "Integer property is incorrect");
            REQUIRE_MESSAGE (tweaksFile.getProperty<juce::String> ("test_string") == newStr, "String property is incorrect");
            REQUIRE_MESSAGE (juce::approximatelyEqual (tweaksFile.getProperty<float> ("test_float"), newFloat), "Float property is incorrect");
        }

        testTweaksFile.deleteFile();
    }

    SECTION ("Tweaks File Listener Test")
    {
        chowdsp::GenericTweaksFile<false> tweaksFile;
        tweaksFile.initialise (testTweaksFile, 1);

        tweaksFile.addProperties ({ { "test_int", 0 }, { "test_string", juce::String {} } });
        tweaksFile.getProperty ("test_float", -1.0f);

        REQUIRE_MESSAGE (tweaksFile.getProperty<int> ("test_int") == 0, "Initial integer property is incorrect");
        REQUIRE_MESSAGE (tweaksFile.getProperty<juce::String> ("test_string") == juce::String {}, "Initial string property is incorrect");
        REQUIRE_MESSAGE (juce::approximatelyEqual (tweaksFile.getProperty<float> ("test_float"), -1.0f), "Initial float property is incorrect");

        static constexpr int newInt = 440;
        static constexpr float newFloat = -110.0f;

        bool listenerHit = false;
        auto callback = tweaksFile.addListener (
            [&listenerHit, &tweaksFile] (const std::string_view& name)
            {
                listenerHit = true;
                if (name == "test_int")
                    REQUIRE_MESSAGE (tweaksFile.getProperty<int> ("test_int") == newInt, "Integer property is incorrect");
                else if (name == "test_float")
                    REQUIRE_MESSAGE (juce::approximatelyEqual (tweaksFile.getProperty<float> ("test_float"), newFloat), "Float property is incorrect");
            });

        const auto jsonConfig = chowdsp::json {
            { "test_int", newInt },
            { "test_float", newFloat },
        };
        chowdsp::JSONUtils::toFile (jsonConfig, testTweaksFile);

        juce::MessageManager::getInstance()->runDispatchLoopUntil (2000);

        REQUIRE_MESSAGE (tweaksFile.getProperty<int> ("test_int") == newInt, "Integer property is incorrect");
        REQUIRE_MESSAGE (juce::approximatelyEqual (tweaksFile.getProperty<float> ("test_float"), newFloat), "Float property is incorrect");
        REQUIRE_MESSAGE (listenerHit, "Tweaks file listener was never hit!");

        testTweaksFile.deleteFile();
    }

    SECTION ("Baked File Test")
    {
        chowdsp::GenericTweaksFile<true> tweaksFile;
        tweaksFile.initialise (BinaryData::test_tweaks_file, BinaryData::test_tweaks_fileSize);

        REQUIRE_MESSAGE (tweaksFile.getProperty<int> ("test_int") == 44, "Integer property is incorrect");
        REQUIRE_MESSAGE (tweaksFile.getProperty<juce::String> ("test_string") == juce::String { "blah blah" }, "String property is incorrect");
        REQUIRE_MESSAGE (juce::approximatelyEqual (tweaksFile.getProperty<float> ("test_float"), 42.0f), "Float property is incorrect");
    }
}
