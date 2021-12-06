#include <TimedUnitTest.h>

namespace
{
NamedValueSet::NamedValue test1 { "test1", 35 };
NamedValueSet::NamedValue test2 { "test2", "TEST" };

const String settingsFile = "settings_file.settings";
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
        expectEquals ((int) settings.getProperty (test1.name), (int) test1.value, "Property 1 is incorrect!");
        expectEquals (settings.getProperty (test2.name).toString(), test2.value.toString(), "Property 2 is incorrect!");

        settings.getSettingsFile().deleteFile();
    }

    void addPropsAfterInit()
    {
        chowdsp::GlobalPluginSettings settings;
        settings.initialise ("settings_file.settings", 1);

        settings.addProperties ({ test1, test2 });
        expectEquals ((int) settings.getProperty (test1.name), (int) test1.value, "Property 1 is incorrect!");
        expectEquals (settings.getProperty (test2.name).toString(), test2.value.toString(), "Property 2 is incorrect!");

        settings.getSettingsFile().deleteFile();
    }

    void noInit()
    {
        chowdsp::GlobalPluginSettings settings;
        expect (settings.getProperty (test1.name).isVoid(), "Property should be void!");
        expect (settings.getSettingsFile() == File(), "Settings file should not exist!");
    }

    void doubleInit()
    {
        chowdsp::GlobalPluginSettings settings;
        settings.initialise (settingsFile, 1);

        settings.addProperties ({ test1, test2 });
        expectEquals ((int) settings.getProperty (test1.name), (int) test1.value, "Property 1 is incorrect!");
        expectEquals (settings.getProperty (test2.name).toString(), test2.value.toString(), "Property 2 is incorrect!");

        settings.initialise (settingsFile, 1);
        expectEquals ((int) settings.getProperty (test1.name), (int) test1.value, "Property 1 is incorrect after 2nd init!");
        expectEquals (settings.getProperty (test2.name).toString(), test2.value.toString(), "Property 2 is incorrect after 2nd init!");

        settings.getSettingsFile().deleteFile();
    }

    void corruptSettingsTest()
    {
        File settingsFileState;
        {
            chowdsp::GlobalPluginSettings settings;
            settings.initialise (settingsFile, 1);
            settings.addProperties ({ test1, test2 });
            settingsFileState = settings.getSettingsFile();
        }

        settingsFileState.replaceWithText (String());

        {
            chowdsp::GlobalPluginSettings settings;
            settings.initialise (settingsFile, 1);
            expect (settings.getProperty (test1.name).isVoid(), "Property should be void!");
            settings.addProperties ({ test1, test2 });
        }

        XmlElement dummyXml ("DUMMY");
        dummyXml.writeTo (settingsFileState);

        {
            chowdsp::GlobalPluginSettings settings;
            settings.initialise (settingsFile, 1);
            expect (settings.getProperty (test1.name).isVoid(), "Property should be void!");
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

            expectEquals ((int) settings.getProperty (test1.name), (int) test1.value, "Property 1 is incorrect after 2nd init!");
            expectEquals (settings.getProperty (test2.name).toString(), test2.value.toString(), "Property 2 is incorrect after 2nd init!");

            settings.getSettingsFile().deleteFile();
        }
    }

    void settingsListenerTest()
    {
        struct TestListener : chowdsp::GlobalPluginSettings::Listener
        {
            int test1Value = 0;
            String test2Value {};

            void propertyChanged (const Identifier& id, const var& v) override
            {
                if (id == test1.name)
                    test1Value = (int) v;
                else if (id == test2.name)
                    test2Value = v.toString();
            }
        } testListener;

        chowdsp::GlobalPluginSettings settings;
        settings.initialise (settingsFile, 1);
        settings.addProperties ({ test1, test2 }, &testListener);

        auto setSettingsVal = [&] (const Identifier& name, var val) {
            auto settingsXml = XmlDocument::parse (settings.getSettingsFile());
            NamedValueSet settingsSet;
            settingsSet.setFromXmlAttributes (*settingsXml);
            settingsSet.set (name, val);
            settingsSet.copyToXmlAttributes (*settingsXml);
            settingsXml->writeTo (settings.getSettingsFile());
            MessageManager::getInstance()->runDispatchLoopUntil (1500);
        };

        constexpr int testVal1 = 22;
        setSettingsVal (test1.name, testVal1);
        expectEquals (testListener.test1Value, testVal1, "Listener value not set!");

        constexpr int testVal2 = 80;
        settings.removePropertyListener (&testListener);
        setSettingsVal (test1.name, testVal2);
        expectEquals (testListener.test1Value, testVal1, "Listener value should not be set after listener removed!");

        const String testStr1 = "RRRRR";
        settings.addPropertyListener (test2.name, &testListener);
        setSettingsVal (test2.name, testStr1);
        expectEquals (testListener.test2Value, testStr1, "Listener value not set!");

        const String testStr2 = "BBBBB";
        settings.removePropertyListener (test2.name, &testListener);
        setSettingsVal (test2.name, testStr2);
        expectEquals (testListener.test2Value, testStr1, "Listener value should not be set after listener removed!");

        settings.getSettingsFile().deleteFile();
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
    }
};

static GlobalSettingsTest globalSettingsTest;
