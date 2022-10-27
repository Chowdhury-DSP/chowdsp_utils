#include <TimedUnitTest.h>
#include <chowdsp_plugin_utils/chowdsp_plugin_utils.h>

namespace
{
const juce::File testConfigFile = juce::File::getSpecialLocation (juce::File::SpecialLocationType::userDesktopDirectory)
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
} // namespace

class ConfigFileTest : public TimedUnitTest
{
public:
    ConfigFileTest() : TimedUnitTest ("Config File Test")
    {
    }

    void writeReadTest (juce::Random& rand)
    {
        chowdsp::experimental::ConfigFile configFile;
        configFile.initialise (testConfigFile, 1);

        configFile.addProperties ({ { "test_int", 0 }, { "test_string", juce::String {} } });

        for (int i = 0; i < 20; ++i)
        {
            const auto newInt = rand.nextInt ({ -100, 100 });
            const auto newStr = getRandomString (rand);

            const auto jsonConfig = chowdsp::json {
                { "test_int", newInt },
                { "test_string", newStr }
            };
            chowdsp::JSONUtils::toFile (jsonConfig, testConfigFile);

            juce::MessageManager::getInstance()->runDispatchLoopUntil (2000);

            expectEquals (configFile.getProperty<int> ("test_int"), newInt, "Integer property is incorrect");
            expectEquals (configFile.getProperty<juce::String> ("test_string"), newStr, "String property is incorrect");
        }

        testConfigFile.deleteFile();
    }

    void runTestTimed() override
    {
        auto random = getRandom();

        beginTest ("Write/Read Test");
        writeReadTest (random);
    }
};

static ConfigFileTest configFileTest;
