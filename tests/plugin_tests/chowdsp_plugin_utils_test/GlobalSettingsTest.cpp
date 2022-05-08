#include <TimedUnitTest.h>
#include <chowdsp_plugin_utils/chowdsp_plugin_utils.h>

using namespace chowdsp::JSONUtils;

namespace
{
chowdsp::GlobalPluginSettings::SettingProperty test1 { "test1", 35 };
chowdsp::GlobalPluginSettings::SettingProperty test2 { "test2", "TEST" };

const juce::String settingsFile = "settings_file.settings";
} // namespace

class GlobalSettingsTest : public TimedUnitTest
{
public:
    GlobalSettingsTest() : TimedUnitTest ("Global Settings Test")
    {
    }

    void addPropsBeforeInit()
    {
        chowdsp::GlobalPluginSettings settings;
        settings.addProperties ({ test1, test2 });

        settings.initialise (settingsFile, 1);
        expectEquals (settings.getProperty<int> (test1.first), test1.second.get<int>(), "Property 1 is incorrect!");
        expectEquals (settings.getProperty<juce::String> (test2.first), test2.second.get<juce::String>(), "Property 2 is incorrect!");

        settings.getSettingsFile().deleteFile();
    }

    void addPropsAfterInit()
    {
        chowdsp::GlobalPluginSettings settings;
        settings.initialise ("settings_file.settings", 1);

        settings.addProperties ({ test1, test2 });
        expectEquals (settings.getProperty<int> (test1.first), test1.second.get<int>(), "Property 1 is incorrect!");
        expectEquals (settings.getProperty<juce::String> (test2.first), test2.second.get<juce::String>(), "Property 2 is incorrect!");

        settings.getSettingsFile().deleteFile();
    }

    void noInit()
    {
        chowdsp::GlobalPluginSettings settings;
        expectEquals (settings.getProperty<int> (test1.first), 0, "Property should be null!");
        expect (settings.getSettingsFile() == juce::File(), "Settings file should not exist!");
    }

    void doubleInit()
    {
        chowdsp::GlobalPluginSettings settings;
        settings.initialise (settingsFile, 1);

        settings.addProperties ({ test1, test2 });
        expectEquals (settings.getProperty<int> (test1.first), test1.second.get<int>(), "Property 1 is incorrect!");
        expectEquals (settings.getProperty<juce::String> (test2.first), test2.second.get<juce::String>(), "Property 2 is incorrect!");

        settings.initialise (settingsFile, 1);
        expectEquals (settings.getProperty<int> (test1.first), test1.second.get<int>(), "Property 1 is incorrect after 2nd init!");
        expectEquals (settings.getProperty<juce::String> (test2.first), test2.second.get<juce::String>(), "Property 2 is incorrect after 2nd init!");

        settings.getSettingsFile().deleteFile();
    }

    void corruptSettingsTest()
    {
        juce::File settingsFileState;
        {
            chowdsp::GlobalPluginSettings settings;
            settings.initialise (settingsFile, 1);
            settings.addProperties ({ test1, test2 });
            settingsFileState = settings.getSettingsFile();
        }

        settingsFileState.replaceWithText (juce::String());

        {
            chowdsp::GlobalPluginSettings settings;
            settings.initialise (settingsFile, 1);
            expectEquals (settings.getProperty<int> (test1.first), 0, "Property should be null!");
            settings.addProperties ({ test1, test2 });
        }

        juce::XmlElement dummyXml ("DUMMY");
        dummyXml.writeTo (settingsFileState);

        {
            chowdsp::GlobalPluginSettings settings;
            settings.initialise (settingsFile, 1);
            expectEquals (settings.getProperty<int> (test1.first), 0, "Property should be null!");
        }

        settingsFileState.deleteFile();
    }

    void persistentSettingsTest()
    {
        {
            chowdsp::GlobalPluginSettings settings;
            settings.initialise (settingsFile, 1);
            settings.addProperties ({ test1, test2 });
        }

        {
            chowdsp::GlobalPluginSettings settings;
            settings.initialise (settingsFile, 1);
            settings.addProperties ({ test1, test2 });

            expectEquals (settings.getProperty<int> (test1.first), test1.second.get<int>(), "Property 1 is incorrect after 2nd init!");
            expectEquals (settings.getProperty<juce::String> (test2.first), test2.second.get<juce::String>(), "Property 2 is incorrect after 2nd init!");

            settings.getSettingsFile().deleteFile();
        }
    }

    void settingsListenerTest()
    {
        struct TestListener : chowdsp::GlobalPluginSettings::Listener
        {
            chowdsp::GlobalPluginSettings* settings = nullptr;
            int test1Value = 0;
            juce::String test2Value {};

            void globalSettingChanged (chowdsp::GlobalPluginSettings::SettingID id) override
            {
                if (id == test1.first)
                    test1Value = settings->getProperty<int> (id);
                else if (id == test2.first)
                    test2Value = settings->getProperty<juce::String> (id);
            }
        } testListener;

        chowdsp::GlobalPluginSettings settings;
        testListener.settings = &settings;

        settings.initialise (settingsFile, 1);
        settings.addProperties ({ test1, test2 }, &testListener);

        auto setSettingsVal = [&] (std::string_view name, const auto& val) {
            auto settingsJson = fromFile (settings.getSettingsFile());
            settingsJson["plugin_settings"][name.data()] = val;

            toFile (settingsJson, settings.getSettingsFile());
            juce::MessageManager::getInstance()->runDispatchLoopUntil (1500);
        };

        constexpr int testVal1 = 22;
        setSettingsVal (test1.first, testVal1);
        expectEquals (testListener.test1Value, testVal1, "Listener value not set!");

        setSettingsVal (test1.first, "testVal2");
        expectEquals (testListener.test1Value, testVal1, "Listener value should not be set with wrong data type!");

        constexpr int testVal2 = 80;
        settings.removePropertyListener (&testListener);
        setSettingsVal (test1.first, testVal2);
        expectEquals (testListener.test1Value, testVal1, "Listener value should not be set after listener removed!");

        const juce::String testStr1 = "RRRRR";
        settings.addPropertyListener (test2.first, &testListener);
        settings.setProperty (test2.first, testStr1);
        expectEquals (testListener.test2Value, testStr1, "Listener value not set!");

        settings.setProperty (test2.first, 45);
        expectEquals (testListener.test2Value, testStr1, "Listener value should not be set when set with the wrong data type!");

        const juce::String testStr2 = "BBBBB";
        settings.removePropertyListener (test2.first, &testListener);
        settings.removePropertyListener ("NOT_A_PROPERTY", &testListener);
        settings.setProperty (test2.first, testStr2);
        settings.setProperty ("NOT_A_PROPERTY", testStr2);
        expectEquals (testListener.test2Value, testStr1, "Listener value should not be set after listener removed!");

        settings.getSettingsFile().deleteFile();
    }

    void setWrongDataTypeTest()
    {
        chowdsp::GlobalPluginSettings settings;
        settings.initialise (settingsFile, 1);
        settings.addProperties ({ test1, test2 });

        expectEquals (settings.getProperty<int> (test1.first), test1.second.get<int>(), "Property 1 is incorrect!");
        expectEquals (settings.getProperty<juce::String> (test2.first), test2.second.get<juce::String>(), "Property 2 is incorrect!");

        settings.setProperty (test1.first, juce::String ("ZZZZ"));
        settings.setProperty (test2.first, 90);

        expectEquals (settings.getProperty<int> (test1.first), test1.second.get<int>(), "Property 1 is incorrect!");
        expectEquals (settings.getProperty<juce::String> (test2.first), test2.second.get<juce::String>(), "Property 2 is incorrect!");
    }

    void wreckSettingsFile()
    {
        {
            chowdsp::GlobalPluginSettings settings;
            settings.initialise (settingsFile, 1);
            settings.addProperties ({ test1, test2 });

            auto settingsJson = fromFile (settings.getSettingsFile());
            settingsJson = chowdsp::json ({});
            toFile (settingsJson, settings.getSettingsFile());
            juce::MessageManager::getInstance()->runDispatchLoopUntil (1500);

            expectEquals (settings.getProperty<int> (test1.first), test1.second.get<int>(), "Property 1 is incorrect!");
            expectEquals (settings.getProperty<juce::String> (test2.first), test2.second.get<juce::String>(), "Property 2 is incorrect!");
        }

        {
            chowdsp::GlobalPluginSettings settings;
            settings.initialise (settingsFile, 1);
            settings.addProperties ({ test1, test2 });

            auto settingsJson = fromFile (settings.getSettingsFile());
            settingsJson = chowdsp::json ({});
            settingsJson["dummy"] = 42;
            toFile (settingsJson, settings.getSettingsFile());
            juce::MessageManager::getInstance()->runDispatchLoopUntil (1500);

            expectEquals (settings.getProperty<int> (test1.first), test1.second.get<int>(), "Property 1 is incorrect!");
            expectEquals (settings.getProperty<juce::String> (test2.first), test2.second.get<juce::String>(), "Property 2 is incorrect!");
        }
    }

    void runTestTimed() override
    {
        // clean up just in case last test didn't finish!
        {
            chowdsp::GlobalPluginSettings settings;
            settings.initialise (settingsFile, 1);
            settings.getSettingsFile().deleteFile();
        }

        beginTest ("Add Properties Before Initialisation");
        addPropsBeforeInit();

        beginTest ("Add Properties After Initialisation");
        addPropsAfterInit();

        beginTest ("No Initialisation");
        noInit();

        beginTest ("Double Initialisation");
        doubleInit();

        beginTest ("Corrupt Settings Test");
        corruptSettingsTest();

        beginTest ("Persistent Settings Test");
        persistentSettingsTest();

        beginTest ("Settings Listener Test");
        settingsListenerTest();

        beginTest ("Set Wrong Data Type Test");
        setWrongDataTypeTest();

        beginTest ("Wreck Settings File Test");
        wreckSettingsFile();
    }
};

static GlobalSettingsTest globalSettingsTest;
