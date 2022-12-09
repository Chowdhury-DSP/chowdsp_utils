#include <TimedUnitTest.h>
#include <chowdsp_plugin_utils/chowdsp_plugin_utils.h>

namespace
{
const juce::File testTweaksFile = juce::File::getSpecialLocation (juce::File::SpecialLocationType::userDesktopDirectory)
                                      .getChildFile ("chowdsp_utils_test_config.json");

constexpr std::string_view randomChars { "0123456789 ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz" };

juce::String getRandomString (juce::Random& rand)
{
    const auto numChars = (int) randomChars.length();
    const auto stringLen = rand.nextInt ({ 10, 50 });

    juce::String str;
    for (int i = 0; i < stringLen; ++i)
    {
        const auto nextChar = randomChars[(size_t) rand.nextInt (numChars)];
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

class TweaksFileTest : public TimedUnitTest
{
public:
    TweaksFileTest() : TimedUnitTest ("Tweaks File Test")
    {
    }

    void writeReadTest (juce::Random& rand)
    {
        chowdsp::GenericTweaksFile<false> tweaksFile;
        tweaksFile.initialise (testTweaksFile, 1);

        tweaksFile.addProperties ({ { "test_int", 0 }, { "test_string", juce::String {} } });
        tweaksFile.getProperty ("test_float", -1.0f);

        expectEquals (tweaksFile.getProperty<int> ("test_int"), 0, "Initial integer property is incorrect");
        expectEquals (tweaksFile.getProperty<juce::String> ("test_string"), juce::String {}, "Initial string property is incorrect");
        expectEquals (tweaksFile.getProperty<float> ("test_float"), -1.0f, "Initial float property is incorrect");

        for (int i = 0; i < 10; ++i)
        {
            const auto newInt = rand.nextInt ({ -100, 100 });
            const auto newStr = getRandomString (rand);
            const auto newFloat = rand.nextFloat();

            const auto jsonConfig = chowdsp::json {
                { "test_int", newInt },
                { "test_string", newStr },
                { "test_float", newFloat },
            };
            chowdsp::JSONUtils::toFile (jsonConfig, testTweaksFile);

            juce::MessageManager::getInstance()->runDispatchLoopUntil (2000);

            expectEquals (tweaksFile.getProperty<int> ("test_int"), newInt, "Integer property is incorrect");
            expectEquals (tweaksFile.getProperty<juce::String> ("test_string"), newStr, "String property is incorrect");
            expectEquals (tweaksFile.getProperty<float> ("test_float"), newFloat, "Float property is incorrect");
        }

        testTweaksFile.deleteFile();
    }

    void tweaksFileListenerTest()
    {
        chowdsp::GenericTweaksFile<false> tweaksFile;
        tweaksFile.initialise (testTweaksFile, 1);

        tweaksFile.addProperties ({ { "test_int", 0 }, { "test_string", juce::String {} } });
        tweaksFile.getProperty ("test_float", -1.0f);

        expectEquals (tweaksFile.getProperty<int> ("test_int"), 0, "Initial integer property is incorrect");
        expectEquals (tweaksFile.getProperty<juce::String> ("test_string"), juce::String {}, "Initial string property is incorrect");
        expectEquals (tweaksFile.getProperty<float> ("test_float"), -1.0f, "Initial float property is incorrect");

        static constexpr int newInt = 440;
        static constexpr float newFloat = -110.0f;

        bool listenerHit = false;
        auto callback = tweaksFile.addListener (
            [this, &listenerHit, &tweaksFile] (const std::string_view& name)
            {
                listenerHit = true;
                if (name == "test_int")
                    expectEquals (tweaksFile.getProperty<int> ("test_int"), newInt, "Integer property is incorrect");
                else if (name == "test_float")
                    expectEquals (tweaksFile.getProperty<float> ("test_float"), newFloat, "Float property is incorrect");
            });

        const auto jsonConfig = chowdsp::json {
            { "test_int", newInt },
            { "test_float", newFloat },
        };
        chowdsp::JSONUtils::toFile (jsonConfig, testTweaksFile);

        juce::MessageManager::getInstance()->runDispatchLoopUntil (2000);

        expectEquals (tweaksFile.getProperty<int> ("test_int"), newInt, "Integer property is incorrect");
        expectEquals (tweaksFile.getProperty<float> ("test_float"), newFloat, "Float property is incorrect");
        expect (listenerHit, "Tweaks file listener was never hit!");

        testTweaksFile.deleteFile();
    }

    void bakedFileTest()
    {
        chowdsp::GenericTweaksFile<true> tweaksFile;
        tweaksFile.initialise (BinaryData::test_tweaks_file, BinaryData::test_tweaks_fileSize);

        expectEquals (tweaksFile.getProperty<int> ("test_int"), 44, "Integer property is incorrect");
        expectEquals (tweaksFile.getProperty<juce::String> ("test_string"), juce::String { "blah blah" }, "String property is incorrect");
        expectEquals (tweaksFile.getProperty<float> ("test_float"), 42.0f, "Float property is incorrect");
    }

    void runTestTimed() override
    {
        auto random = getRandom();

        beginTest ("Write/Read Test");
        writeReadTest (random);

        beginTest ("Tweaks File Listener Test");
        tweaksFileListenerTest();

        beginTest ("Baked File Test");
        bakedFileTest();
    }
};

static TweaksFileTest tweaksFileTest;
